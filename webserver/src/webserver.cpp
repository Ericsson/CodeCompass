#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/attributes.hpp>
#include <boost/program_options.hpp>

#include <util/logutil.h>
#include <util/webserverutil.h>

#include "threadedmongoose.h"
#include "mainrequesthandler.h"

namespace po = boost::program_options;
namespace trivial = boost::log::trivial;

po::options_description commandLineArguments()
{
  po::options_description desc("CodeCompass options");

  desc.add_options()
    ("help,h",
      "Prints this help message.")
    ("workspace,w", po::value<std::string>()->required(),
      "Path to a workspace file which contains information of the parsed "
      "project in an INI-like format.")
    ("database,d", po::value<std::string>()->required(),
      // TODO: Provide a full connection string example.
      "A connection string to the relational database with the following "
      "format: pgsql:database=name;user=user_name.")
    ("port,p", po::value<int>()->default_value(8080),
      "Port number of the webserver to listen on.")
    ("loglevel",
      po::value<trivial::severity_level>()->default_value(trivial::info),
      "Logging level of the parser. Possible values are: debug, info, warning, "
      "error, critical")
    ("threads,t", po::value<int>()->default_value(4),
      "Number of worker threads.");

  return desc;
}

int main(int argc, char* argv[])
{
  std::string binDir = boost::filesystem::canonical(
    boost::filesystem::path(argv[0]).parent_path()).string();

  cc::util::initLogger();
 
  cc::webserver::MainRequestHandler requestHandler;
  requestHandler.pluginHandler.addDirectory(binDir + "/../lib/serviceplugin");

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
    BOOST_LOG_TRIVIAL(error) << "Error in command line arguments: " << e.what();
    return 1;
  }

  vm.insert(std::make_pair("binDir", po::variable_value(binDir, false)));
  vm.insert(std::make_pair("webguiDir", po::variable_value(
    binDir + "/../share/codecompass/webgui/", false)));
  vm.insert(std::make_pair("workspaceCfgFile", po::variable_value(
    vm["workspace"].as<std::string>() + "/workspace.cfg", false)));

  //--- Process workspaces ---//

  cc::util::WorkspaceOptions workspaceOptions
    = cc::util::parseConfigFile(vm["workspaceCfgFile"].as<std::string>());

  for (const auto& ws : workspaceOptions)
  {
    const cc::util::WorkspaceOption& wsOpt = ws.second;

    BOOST_LOG_TRIVIAL(info)
      << "Workspace" << std::endl
      << "  id = " << ws.first << std::endl
      << "  connection = " << wsOpt.connectionString << std::endl
      << "  datadir = " << wsOpt.datadir << std::endl
      << "  description = " << wsOpt.description << std::endl;
  }

  requestHandler.pluginHandler.configure(vm);

  //--- Start mongoose server ---//

  cc::webserver::ThreadedMongoose server(vm["threads"].as<int>());
  server.setOption("listening_port", std::to_string(vm["port"].as<int>()));
  server.setOption("document_root", vm["webguiDir"].as<std::string>());

  BOOST_LOG_TRIVIAL(info) 
    << "Mongoose web server starting on port "
    << server.getOption("listening_port");

  try
  {
    server.run(requestHandler);
    BOOST_LOG_TRIVIAL(info) << "Exiting, waiting for all threads to finish...";
  }
  catch (const std::exception& ex)
  {
    BOOST_LOG_TRIVIAL(error) << "Exited with exception: " << ex.what();
    return 1;
  }

  return 0;
}
