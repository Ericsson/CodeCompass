#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <util/logutil.h>
#include <util/webserverutil.h>

#include "threadedmongoose.h"
#include "mainrequesthandler.h"
#include "config.h"

namespace po = boost::program_options;

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
    ("threads,t", po::value<int>()->default_value(4),
      "Number of worker threads.");

  return desc;
}

int main(int argc, char* argv[])
{
  cc::util::initLogger();
 
  cc::webserver::MainRequestHandler requestHandler;
  requestHandler.pluginHandler.addDirectory(WEBSERVER_PLUGIN_DIR);

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

  try
  {
    po::notify(vm);
  }
  catch (const po::error& e)
  {
    BOOST_LOG_TRIVIAL(error) << "Error in command line arguments: " << e.what();
    return 1;
  }

  if (vm.count("workspace"))
  {
    cc::util::WorkspaceOptions workspaceOptions
      = cc::util::parseConfigFile(vm["workspace"].as<std::string>());

    for (const auto& ws : workspaceOptions)
    {
      const cc::util::WorkspaceOption& wsOpt = ws.second;

      BOOST_LOG_TRIVIAL(info)
        << "Workspace" << std::endl
        << "  id = " << ws.first << std::endl
        << "  connection = " << wsOpt.connectionString << std::endl
        << "  description = " << wsOpt.description << std::endl;
    }
  }

  requestHandler.pluginHandler.configure(vm);

  //--- Start mongoose server ---//

  cc::webserver::ThreadedMongoose server(vm["threads"].as<int>());
  server.setOption("listening_port", std::to_string(vm["port"].as<int>()));

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
