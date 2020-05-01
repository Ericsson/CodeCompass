// Ensure that Mongoose API is defined the same way in all translation units!
#include <webserver/mongoose.h> // IWYU pragma: keep

#include <functional>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <util/filesystem.h>
#include <util/logutil.h>
#include <util/threadpool.h>
#include <util/webserverutil.h>

#include <webserver/httprequest.h>

#include "authentication.h"
#include "mainrequesthandler.h"
#include "mongooseserver.h"
#include "sessionmanager.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace trivial = boost::log::trivial;

using namespace cc::util;
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
         "Number of worker threads.");

    return desc;
}

int main(int argc, char* argv[])
{
    std::string compassRoot = binaryPathToInstallDir(argv[0]);

    const std::string AUTH_PLUGIN_DIR = compassRoot + "/lib/authplugin";
    const std::string SERVICE_PLUGIN_DIR = compassRoot + "/lib/serviceplugin";
    const std::string WEBGUI_DIR = compassRoot + "/share/codecompass/webgui/";

    initLogger();

    MainRequestHandler requestHandler;
    requestHandler.pluginHandler.addDirectory(SERVICE_PLUGIN_DIR);

    //--- Process command line arguments ---//

    po::options_description desc = commandLineArguments();

    po::options_description pluginOptions
        = requestHandler.pluginHandler.getOptions();
    desc.add(pluginOptions);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

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

    //--- Process workspaces ---//

    ServerContext ctx(compassRoot, vm, sessions.get());
    requestHandler.pluginHandler.configure(ctx);
    requestHandler.updateServedPathsFromPluginHandler();

    //--- Start mongoose server ---//

#if MG_ENABLE_SSL
    // Check if certificates exist in the workspace - if so, start SSL.
    auto certPath = fs::path(vm["workspace"].as<std::string>())
        .append("server.pem").native();
    auto keyPath = fs::path(vm["workspace"].as<std::string>())
        .append("server.key").native();
    if (fs::is_regular_file(certPath) && fs::is_regular_file(keyPath))
        LOG(info) << "Starting HTTPS listener with certificate 'server.pem' "
                     "and private key 'server.key'.";
    else
    {
        LOG(warning)
          << "No 'server.pem' or 'server.key' found in '--workspace', server "
             "running over conventional HTTP!";
      certPath = keyPath = "";
    }
#else
    LOG(warning) << "CodeCompass server built without SSL support. Running over "
                    "conventional HTTP!";
    std::string certPath, keyPath;
#endif // MG_ENABLE_SSL

    std::string listenPort = std::to_string(vm["port"].as<int>());
    LOG(info) << "Mongoose web server starting on port " << listenPort;
    MongooseServer server{std::move(listenPort),
                          WEBGUI_DIR,
                          std::move(certPath),
                          std::move(keyPath)};
    server.setHTTPServiceURIPaths(&requestHandler.servedPaths);

    //--- Create the worker threads that generate responses ---//

    std::unique_ptr<JobQueueThreadPool<HTTPRequest>> threads =
        make_thread_pool<HTTPRequest>(
            vm["jobs"].as<int>(),
            [&server, &requestHandler](HTTPRequest& req_) {
                // The executor object of the worker thread is to handle the
                // request itself by the main handler dispatching it to the
                // specific handler implementation.
                requestHandler(req_);
                server.wakeUp();
            },
            true);

    // The callback function of the server itself is to push a received and
    // understood request to the worker threads' queue.
    server.setHTTPServiceRequestHandler(
        [pool = threads.get()](HTTPRequest&& req_)
        {
            pool->enqueue(std::move(req_));
        });

    //--- Begin listening ---//

    try
    {
        server.loop();
        LOG(info) << "Exiting listener! Waiting for all threads to finish...";
    }
    catch (const std::exception& ex)
    {
        LOG(error) << "Exited with exception: " << ex.what();
        return 1;
    }

  return 0;
}
