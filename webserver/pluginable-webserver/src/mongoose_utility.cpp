#include <algorithm>
#include <iterator>
#include <string>
#include <tuple>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include <mongoose/plugin.h>
#include <mongoose/mongoose_utility.h>

namespace
{

/**
 * Parses a workspace configuration file key for real key and wsid.
 *
 * For example:
 *  for key workspace.proj1.connection it return the (proj1, connection) tuple.
 *
 * @param key_ a key from a workspace configuration file as it's in the
 *             parsed_options
 * @return It returns a wsid, key pair.
 */
std::tuple<std::string, std::string> parseWorkspaceCfgOption(
  const std::string& key_)
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
    BOOST_LOG_TRIVIAL(error)
      << "Error parsing workspace parameters: " << ex.what();
    throw;
  }
}

}

namespace cc 
{ 
namespace mongoose 
{
  
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

void parseConfiguration(
  const boost::program_options::options_description& options_,
  int argc_, char **argv_,
  boost::program_options::variables_map& varMap_, bool allowUnkown_)
{
  namespace po = ::boost::program_options;

  if (allowUnkown_)
  {
    po::parsed_options parsed = po::command_line_parser(argc_, argv_).options(
      options_).allow_unregistered().run();
    store(parsed, varMap_);
  } else
  {
    po::store(po::parse_command_line(argc_, argv_, options_), varMap_);
  }

  if (varMap_.count("config_file"))
  {
    store(
      po::parse_config_file<char>(
        varMap_["config_file"].as<std::string>().c_str(), options_, allowUnkown_),
      varMap_);
  }

  if (varMap_.count("workspaces"))
  {
    parseWorkspacesConfigFile(
      varMap_["workspaces"].as<std::string>().c_str(),
      varMap_);
  }

  notify(varMap_);
}

std::string getContent(mg_connection *conn_)
{
  return std::string(conn_->content, conn_->content + conn_->content_len);
}

std::string getCurrentDate()
{
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  char output[42];
  std::strftime(output, 42, "%c", timeinfo);

  return {output};
}

} // mongoose
} // cc


