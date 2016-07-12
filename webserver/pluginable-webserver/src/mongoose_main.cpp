#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <util/logutil.h>
#include <util/db/dbutil.h>
#include <util/db/odbtransaction.h>
#include <odb/database.hxx>
#include <model/option.h>
#include <model/option-odb.hxx>

#include <iostream>

#include <mongoose/threadedmongoose.h>
#include <mongoose/main_request_handler.h>

namespace po = boost::program_options;

using namespace cc::mongoose;
using namespace cc::plugin;

boost::program_options::options_description getCoreOptions()
{
  namespace po = ::boost::program_options;
  po::options_description coreOptions("Core Options");

  coreOptions.add_options()
    ("help,h", "produce help message")
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
    ("document_root,d", po::value<std::string>(),
      "A directory to serve. By default, current directory is served. Current"
      " directory is commonly referenced as dot (.).")
    ;

  return mongooseOptions;
}


std::string getDataDirFromDb(const std::string& connectionString)
{
  std::cout << connectionString << std::endl;
  std::shared_ptr<odb::database> db
    = cc::util::createDatabase(connectionString);

  if (!db)
  {
    std::cout
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

std::tuple<std::string, std::string>
parseWorkspaceCfgOption(const std::string& key_)
{
  const std::string prefix = "workspace.";

  std::string key = key_;
  if (key.find(prefix) != 0)
  {
    throw std::runtime_error(
     "Unknown option in workspace config: " + key_);
  }
  key = key.substr(prefix.size());

  std::size_t wsNameEnd = key.find('.');
  if (wsNameEnd == key.npos)
  {
    throw std::runtime_error(
     "Bad workspace name in workspace config: " + key_);
  }

  return std::make_tuple(key.substr(0, wsNameEnd), key.substr(wsNameEnd+1));
}

/**
 * Parses the inside parameters of a workspace.
 *
 * @param options_ container of workspaces for store the new workspace.
 * @param values_ the vector of inside parameters
 */
void parseWorkspaceCfgValues(
  cc::mongoose::WorkspaceOptions& options_,
  const std::vector<std::string>& values_)
{
  namespace po = boost::program_options;

  cc::mongoose::WorkspaceOption wsOpt;
  po::options_description optDesc("Workspace Options");
  optDesc.add_options()
    ("id",
      po::value<std::string>(&wsOpt.workspaceId)->required(),
      "Workspace id")
    ("connection",
      po::value<std::string>(&wsOpt.connectionString)->required(),
      "Connection string")
    ("description",
      po::value<std::string>(&wsOpt.description)->default_value(""),
      "Workspace description")
    ("datadir",
      po::value<std::string>(&wsOpt.dataDir)->default_value(""),
      "Data direcory (same as -a parameter of parse command)")
    ("codecheckerurl",
      po::value<std::string>(&wsOpt.codeCheckerUrl)->default_value(""),
      "URL of the CodeChecker service. E.g. http://localhost:9999/CodeCheckerService")
    ("codecheckerrunid",
      po::value<std::string>(&wsOpt.codeCheckerRunId)->default_value(""),
      "The id of the CodeChecker run to use")
    ("codecheckerrunname",
      po::value<std::string>(&wsOpt.codeCheckerRunName)->default_value(""),
      "The name of the CodeChecker run to use")    
    ;

  std::stringstream cfgFile;
  for (const std::string& val : values_)
  {
    cfgFile << val << std::endl;
  }

  try
  {
    po::variables_map vars;
    po::store(po::parse_config_file(cfgFile, optDesc, false), vars);
    po::notify(vars);

    if (wsOpt.searchDir.empty() && !wsOpt.dataDir.empty())
    {
      // If no searchdir but there is datadir than searchdir is $datadir/search
      wsOpt.searchDir = wsOpt.dataDir + "/search";
    }

    options_.push_back(wsOpt);
  }
  catch (std::exception& ex)
  {
    std::cerr << "Error parsing workspace parameters: "
              << ex.what() << std::endl;
    throw;
  }
}

/**
 * Parses a workspace configuration file.
 *
 * @param filePath_ config file path.
 * @param varMap_ variable map for storing workspaces
 */
void parseWorkspacesConfigFile(
  const std::string&                     filePath_,
  boost::program_options::variables_map& varMap_)
{
  namespace po = boost::program_options;

  po::options_description emptyopts;
  auto rawOptions = po::parse_config_file<char>(
    filePath_.c_str(), emptyopts, true);

  std::map<std::string, std::vector<std::string>> workspaces;

  // Iterate over options
  for (const auto& opt : rawOptions.options)
  {
    std::string wsid, optname;

    std::tie(wsid, optname) = parseWorkspaceCfgOption(opt.string_key);
    if (opt.value.size() != 1)
    {
      throw std::runtime_error(
        "Bad value for " + opt.string_key + " in workspace config!");
    }

    workspaces[wsid].emplace_back(optname + "=" + opt.value[0]);
  }

  // Get workspace vector or create it if not esxists.
  po::variables_map::iterator it = varMap_.find(WORKSPACE_OPTION_NAME);
  if (it == varMap_.end())
  {
    it = varMap_.insert(po::variables_map::value_type(
      WORKSPACE_OPTION_NAME,
      po::variable_value(boost::any(WorkspaceOptions()), false))).first;
  }

  // Parse the values
  WorkspaceOptions& parsedOpts = it->second.as<WorkspaceOptions>();
  for (auto& ws : workspaces)
  {
    // In config files the id is in the group name, so we have to add to the
    // inside argumet vector
    ws.second.push_back("id=" + ws.first);
    parseWorkspaceCfgValues(parsedOpts, ws.second);
  }
}

void parseConfiguration(const boost::program_options::options_description&
  options, int argc, char **argv,
  boost::program_options::variables_map& varMap, bool allowUnkown
  )
{
  namespace po = ::boost::program_options;

  if (allowUnkown)
  {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
      options).allow_unregistered().run();
    store(parsed, varMap);
  } else
  {
    po::store(po::parse_command_line(argc, argv, options), varMap);
  }


  if (varMap.count("workspaces"))
  {
    parseWorkspacesConfigFile(
      varMap["workspaces"].as<std::string>().c_str(),
      varMap);
  }

  notify(varMap);
}

int main(int argc, char* argv[])
{
  const bool allowUnregistered = true;
  
  cc::util::initLogger();  
  
  //--- Process command line arguments ---//

  po::options_description desc("Options");
  po::options_description coreOptions = getCoreOptions();
  po::options_description mongooseOptions = getMongooseOptions();

  desc.add(coreOptions);
  desc.add(mongooseOptions);

  po::variables_map vm;
  if(allowUnregistered)
  {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
        desc).allow_unregistered().run();
    po::store(parsed, vm);
  }
  else
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  }
  
  if (argc < 2 || vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }
      
  if (vm.count("workspaces"))
  {
    parseWorkspacesConfigFile(
      vm["workspaces"].as<std::string>().c_str(), vm);
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
  
  MainRequestHandler requestHandler;
  
  
  if (vm.count("workspace"))
  {
    const auto& wses = vm["workspace"].
      as<cc::mongoose::WorkspaceOptions>();

    std::cout << std::endl;
    for (const auto& ws : wses)
    {
      std::cout << "Workspace" << std::endl;
      std::cout << "  id = " << ws.workspaceId << std::endl;
      std::cout << "  connection = " << ws.connectionString << std::endl;
      std::cout << "  description = " << ws.description << std::endl;

      std::string dataDir
        = ws.dataDir.empty()
        ? getDataDirFromDb(ws.connectionString)
        : ws.dataDir;

      if (!dataDir.empty())
      {
        std::cout << "  datadir = " << dataDir << std::endl;
        requestHandler.dataDir[ws.workspaceId] = dataDir;
      }

      if (!ws.searchDir.empty())
        std::cout << "  searchdir = " << ws.searchDir << std::endl;

      std::cout << std::endl;
    }
  }
  
  //--- Request Handler params ---//)
  if (vm.count("document_root"))
  {
    requestHandler.documentRoot = vm["document_root"].as<std::string>();
  }
  
  //--- Plugin settings ---//

  if (vm.count("plugin_dir"))
  {
    boost::filesystem::path pluginDir = vm["plugin_dir"].as<std::string>();
    requestHandler.pluginHandler.addDirectory(pluginDir);

    po::options_description pluginOptions = requestHandler.pluginHandler.getOptions();
    desc.add(pluginOptions);

    // This time all options have to be known
    vm.clear();
    parseConfiguration(desc, argc, argv, vm, false);
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

  std::cout << "Mongoose web server v. " <<  ", starting on port(s) " << server.getOption("listening_port")
    << " with web root [" << server.getOption("document_root") << "]"
    << std::endl;
    
  try
  {
    server.run(requestHandler);
    std::cout << "Exiting, waiting for all threads to finish..." << std::endl;    
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Exited with exception: " << ex.what() << std::endl;
    return 1;
  }
  
  return 0;
}