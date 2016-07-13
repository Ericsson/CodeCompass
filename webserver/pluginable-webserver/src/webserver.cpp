#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <util/logutil.h>
#include <util/db/dbutil.h>
#include <util/db/odbtransaction.h>

#include <odb/database.hxx>
#include <model/option.h>
#include <model/option-odb.hxx>

#include <mongoose/threadedmongoose.h>
#include <mongoose/main_request_handler.h>
#include <mongoose/mongoose_utility.h>

namespace po = boost::program_options;

using namespace cc::mongoose;
using namespace cc::plugin;

boost::program_options::options_description getCoreOptions()
{
  namespace po = ::boost::program_options;
  po::options_description coreOptions("Core Options");

  coreOptions.add_options()
    ("help,h", "produce help message")
    ("config_file,C", po::value<std::string>(), "configuration file")
    ("plugin_dir,P", po::value<std::string>(), "Plugin directory")
    ("workspaces", po::value<std::string>(),
      "configuration file for workspaces");

  return coreOptions;
}

boost::program_options::options_description getMongooseOptions()
{
  namespace po = ::boost::program_options;
  po::options_description mongooseOptions("Mongoose Options");

  mongooseOptions.add_options()
    ("listening_port,p", po::value<std::string>()->default_value("8080"),
      "Comma-separated list of ports to listen on. If the port is SSL, a "
      "letter s must be appeneded, for example, 80,443s will open port 80 "
      "and port 443, and connections on port 443 will be SSL-ed. For non-SSL "
      "ports, it is allowed to append letter r, meaning 'redirect'. Redirect "
      "ports will redirect all their traffic to the first configured SSL port."
      " For example, if listening_ports is 80r,443s, then all HTTP traffic "
      "coming at port 80 will be redirected to HTTPS port 443.")
    ("threads,t", po::value<int>(),
      "Number of worker threads. Mongoose handles each incoming connection"
      " in a separate thread. Therefore, the value of this option is"
      " effectively a number of concurrent HTTP connections"
      " Mongoose can handle.")
    ;

  return mongooseOptions;
}


std::string getDataDirFromDb(const std::string& connectionString)
{
  std::shared_ptr<odb::database> db
    = cc::util::createDatabase(connectionString);

  if (!db)
  {
    BOOST_LOG_TRIVIAL(error)
      << "Wrong connection string: '" << connectionString << "' "
      << "for parser: 'doctoolparser'";

    throw std::runtime_error("Wrong database!");
  }

  return cc::util::OdbTransaction(db)([&db]() -> std::string {
    odb::result<cc::model::Option> options
      = db->query<cc::model::Option>(odb::query<cc::model::Option>());
    
    for (const cc::model::Option& option : options)
      if (option.key == "projectDataDir")
        return option.value;

    return "";
  });
}

int main(int argc, char* argv[])
{  
  cc::util::initLogger();  
  
  //--- Process command line arguments ---//

  po::options_description desc("Options");
  po::options_description coreOptions = getCoreOptions();
  po::options_description mongooseOptions = getMongooseOptions();

  desc.add(coreOptions);
  desc.add(mongooseOptions);

  po::variables_map vm;
  
  if (argc < 2 || vm.count(HELP))
  {
    BOOST_LOG_TRIVIAL(info) << desc;
    return 0;
  }
  
  // At first we permit unknown options
  try
  {
    parseConfiguration(desc, argc, argv, vm, true);
  }
  catch (const std::exception& ex)
  {
    BOOST_LOG_TRIVIAL(error) << "Exited with exception: " << ex.what();
    return 1;
  }
    
  MainRequestHandler requestHandler;
    
  if (vm.count(WORKSPACE_OPTION_NAME))
  {
    const auto& wses = vm[WORKSPACE_OPTION_NAME].
      as<cc::mongoose::WorkspaceOptions>();

    for (const auto& ws : wses)
    {
      std::string dataDir
        = ws.dataDir.empty()
        ? getDataDirFromDb(ws.connectionString)
        : ws.dataDir;
      
      BOOST_LOG_TRIVIAL(info)
        << "Workspace" << std::endl
        << "  id = " << ws.workspaceId << std::endl
        << "  connection = " << ws.connectionString << std::endl
        << "  description = " << ws.description << std::endl
        << "  datadir = " << dataDir << std::endl
        << "  searchdir = " << ws.searchDir;

      if (!dataDir.empty())
      {
        requestHandler.dataDir[ws.workspaceId] = dataDir;
      }
    }
  }
  
  //--- Plugin settings ---//

  if (vm.count(PLUGIN_DIR))
  {
    boost::filesystem::path pluginDir = vm[PLUGIN_DIR].as<std::string>();
    requestHandler.pluginHandler.addDirectory(pluginDir);

    po::options_description pluginOptions = requestHandler.pluginHandler.getOptions();
    desc.add(pluginOptions);

    // This time all options have to be known
    vm.clear();
    try
    {
      parseConfiguration(desc, argc, argv, vm, false);
    }
    catch (const std::exception& ex)
    {
      BOOST_LOG_TRIVIAL(error) << "Exited with exception: " << ex.what();
      return 1;
    }
  }
  
  requestHandler.pluginHandler.configure(vm);
    
  //--- Start mongoose server ---//
    
  cc::mongoose::ThreadedMongoose server;
  for (auto optionDescription : mongooseOptions.options())
  {
    auto longName = optionDescription->long_name();
    if (vm.count(longName)) {
      auto value = vm[longName].as<std::string>();

      server.setOption(longName, value);
    }
  }

  BOOST_LOG_TRIVIAL(info) 
    << "Mongoose web server starting on port(s) "
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