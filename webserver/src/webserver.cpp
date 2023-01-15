#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <util/filesystem.h>
#include <util/logutil.h>
#include <util/webserverutil.h>

#include "authentication.h"
#include "mainrequesthandler.h"
#include "sessionmanager.h"
#include "threadedmongoose.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace trivial = boost::log::trivial;

using namespace cc::webserver;

po::options_description commandLineArguments()
{
    po::options_description desc("CodeCompass options");

    desc.add_options()
        ("help,h",
         "Prints this help message.")
        ("workspace,w", po::value<std::string>()->required(),
         "Path to a workspace directory which contains the parsed projects.")
        ("port,p", po::value<int>()->default_value(8080),
         "Port number of the webserver to listen on.")
        ("loglevel",
         po::value<trivial::severity_level>()->default_value(trivial::info),
         "Logging level of the parser. Possible values are: debug, info, warning, "
         "error, critical")
        ("jobs,j", po::value<int>()->default_value(4),
         "Number of worker threads.")
        ("log-target", po::value<std::string>(),
         "This is the path to the folder where the logging output files will be written. "
         "If omitted, the output will be on the console only.");

    return desc;
}

int main(int argc, char* argv[])
{
    std::string compassRoot = cc::util::binaryPathToInstallDir(argv[0]);

    const std::string AUTH_PLUGIN_DIR = compassRoot + "/lib/authplugin";
    const std::string SERVICE_PLUGIN_DIR = compassRoot + "/lib/serviceplugin";
    const std::string WEBGUI_DIR = compassRoot + "/share/codecompass/webgui/";

    cc::util::initConsoleLogger();

    MainRequestHandler requestHandler;
    requestHandler.pluginHandler.addDirectory(SERVICE_PLUGIN_DIR);

    //--- Process command line arguments ---//

    po::options_description desc = commandLineArguments();

    po::options_description pluginOptions
        = requestHandler.pluginHandler.getOptions();
    desc.add(pluginOptions);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("log-target"))
    {
    vm.at("log-target").value() = cc::util::getLoggingBase( vm["log-target"].as<std::string>()
                                                          , "CodeCompass"
                                                          );

        if (!cc::util::initFileLogger(vm["log-target"].as<std::string>() + "webserver.log"))
        {
            vm.at("log-target").value() = std::string("");
        }
    }

    if (argc < 2 || vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("loglevel"))
    {
        trivial::severity_level loglevel
            = vm["loglevel"].as<trivial::severity_level>();
        boost::shared_ptr<boost::log::core> logger = boost::log::core::get();
        logger->set_filter(boost::log::expressions::attr<
                trivial::severity_level>("Severity") >= loglevel);
        logger->add_global_attribute("Severity",
                boost::log::attributes::mutable_constant<trivial::severity_level>(loglevel));
    }

    try
    {
        po::notify(vm);
    }
    catch (const po::error& e)
    {
        LOG(error) << "Error in command line arguments: " << e.what();
        return 1;
    }

    vm.insert(std::make_pair("webguiDir", po::variable_value(WEBGUI_DIR, false)));

    //--- Set up authentication and session management ---//

    boost::optional<Authentication> authHandler{Authentication{}};
    auto authCfgPath = boost::filesystem::path(
        vm["workspace"].as<std::string>()).append("authentication.json");
    if (boost::filesystem::is_regular_file(authCfgPath))
    {
        try
        {
            authHandler.emplace(AUTH_PLUGIN_DIR, authCfgPath);
        }
        catch (int)
        {
            return 1;
        }
    }

    std::unique_ptr<SessionManager> sessions{
        std::make_unique<SessionManager>(authHandler.get_ptr())};
    requestHandler.sessionManager = sessions.get();

    //--- Set up Google Analytics monitoring---//

    auto gaFilePath = boost::filesystem::path(
      vm["workspace"].as<std::string>()).append("ga.txt");
    if (boost::filesystem::is_regular_file(gaFilePath))
    {
      requestHandler.gaTrackingIdPath = gaFilePath.string();
      LOG(info) << "Google Analytics monitoring enabled.";
    }
    else
    {
      LOG(debug) << "Google Analytics monitoring disabled.";
    }

    //--- Process workspaces ---//

    cc::webserver::ServerContext ctx(compassRoot, vm, sessions.get());
    requestHandler.pluginHandler.configure(ctx);

    //--- Start mongoose server ---//

    cc::webserver::ThreadedMongoose server(vm["jobs"].as<int>());
    server.setOption("listening_port", std::to_string(vm["port"].as<int>()));
    server.setOption("document_root", vm["webguiDir"].as<std::string>());

    // Check if certificate.pem exists in the workspace - if so, start SSL.
    auto certPath = fs::path(vm["workspace"].as<std::string>())
        .append("certificate.pem");
    if (boost::filesystem::is_regular_file(certPath))
    {
        LOG(info) << "Starting HTTPS listener based on 'certificate.pem'.";
        server.setOption("ssl_certificate", certPath.native());
    }
    else
    {
        LOG(warning) << "No 'certificate.pem' found in '--workspace', server "
            "running over conventional HTTP!";
    }

    LOG(info) << "Mongoose web server starting on port "
        << server.getOption("listening_port");

    try
    {
        server.run(requestHandler);
        LOG(info) << "Exiting, waiting for all threads to finish...";
    }
    catch (const std::exception& ex)
    {
        LOG(error) << "Exited with exception: " << ex.what();
        return 1;
    }

  return 0;
}
