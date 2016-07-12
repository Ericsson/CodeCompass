/*
 * mongoose_utility.cpp
 *
 *  Created on: Mar 26, 2013
 *      Author: ezoltbo
 */

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <tuple>
#include <fstream>

#include <boost/algorithm/string.hpp>
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

}

namespace cc 
{ 
namespace mongoose 
{

/**
 * Parser function for workspace structure. For documentation of parameters see
 * the Boost::ProgramOptions documentation.
 */
void validate(
  boost::any& val_,
  const std::vector<std::string>& values_,
  WorkspaceOptions*, int)
{
  if (val_.empty())
  {
    val_ = WorkspaceOptions();
  }

  WorkspaceOptions& options = boost::any_cast<WorkspaceOptions&>(val_);
  if (values_.size() == 1)
  {
    // For config file line
    parseWorkspaceCfgValues(
      options, boost::program_options::split_unix(values_.at(0)));
  }
  else
  {
    // Normal command line
    parseWorkspaceCfgValues(options, values_);
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

boost::program_options::options_description getCoreOptions()
{
  namespace po = ::boost::program_options;
  po::options_description coreOptions("Core Options");

  coreOptions.add_options()
    ("help,h", "produce help message")
    ("config_file,C", po::value<std::string>(), "configuration file")
    ("plugin_dir,P", po::value<std::string>(), "Plugin directory")
    ("loglevel,l", po::value<std::string>()->default_value("info"), "log level (debug/info/warning/error/critical)")
    (WORKSPACE_OPTION_NAME, po::value<WorkspaceOptions>()->multitoken(),
      "Workspace id, database and description connection tuple. For example: " \
      "--workspace "\
        "id=ws1 " \
        "description=\"Tiny XML\" " \
        "database=sqlite:database=/tmp/tiny.sqlite")
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
    ("num_threads,t", po::value<int>(),
      "Number of worker threads. Mongoose handles each incoming connection"
      " in a separate thread. Therefore, the value of this option is"
      " effectively a number of concurrent HTTP connections"
      " Mongoose can handle.")
    ("global_auth_file,g", po::value<std::string>(),
      "Location of a global passwords file.")
    ("auth_domain,R", po::value<std::string>()->default_value("CodeCompass"),
      "Authorization realm.")
    ;

  return mongooseOptions;
}

boost::program_options::options_description getAuthOptions()
{
  namespace po = ::boost::program_options;
  po::options_description authOptions("Authentication Options");

  authOptions.add_options()
    ("auth_mode,a", po::value<std::string>(),
      "If authentication mode is given then user authentication will be"
      " required for using CodeCompass. Possible values are: digest, ldap.")
    ("auth_file,f", po::value<std::string>()->default_value("usertokens"),
      "This flag has to be given a file name. This file"
      " will contain the user IP addresses and user tokens, and in case of"
      " ldap authentication user tokens and user names are stored here.")
    ("ldap_url", po::value<std::string>(),
      "If this flag is given then LDAP authentication will be required for"
      " loading the page. This flag has to be given the full ldap URL, e.g."
      " ldaps://my_ldap_server.com:636. In case of LDAP authentication the"
      " user node's distinguished name (DN) also has to be given. See dn flag.")
    ("ldap_dn", po::value<std::string>(),
      "Distinguished name of the user node in the LDAP database. The username"
      "has to be substituted with {}, e.g. uid={},ou=A,ou=B,o=C")
    ("ldap_cert", po::value<std::string>(),
      "Path to certificate file (*.pam) for TLS/SSL connection.")
    ("stat", po::value<std::string>()->default_value("userstat"),
      "This flag has to be given a file name in which user statistics are"
      " stored. If this flag is not set then no statistics will be made.")
    ;

  return authOptions;
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

  if (varMap.count(CONFIG_FILE))
  {
    store(
      po::parse_config_file<char>(
        varMap[CONFIG_FILE].as<std::string>().c_str(), options, allowUnkown),
      varMap);
  }

  if (varMap.count("workspaces"))
  {
    parseWorkspacesConfigFile(
      varMap["workspaces"].as<std::string>().c_str(),
      varMap);
  }

  notify(varMap);
}

char **createCArrayOfOptions(const ::boost::program_options::variables_map&
  varMap, const ::boost::program_options::options_description& options)
{
  std::vector<std::string> keysAndValues;
  for (auto optionDescription : options.options())
  {
    const std::string& optionKey = optionDescription->long_name();
    if (varMap.count(optionKey))
    {
      std::string valueString;
      const boost::any& value = varMap[optionKey].value();

      if (value.type() == typeid(int))
      {
        valueString = ::boost::lexical_cast<std::string>(
          ::boost::any_cast<int>(value));
      } else if (value.type() == typeid(std::string))
      {
        valueString = ::boost::any_cast<std::string>(value);
      } else
      {
        throw std::runtime_error("Unknown type of " + optionKey);
      }

      keysAndValues.push_back(optionKey);
      keysAndValues.push_back(valueString);
    }
  }

  char **ret = new char*[keysAndValues.size() + 1];

  for (unsigned i = 0; i < keysAndValues.size(); ++i)
  {
    const auto& str = keysAndValues[i];
    ret[i] = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), ret[i]);

    ret[i][str.size()] = '\0';
  }

  ret[keysAndValues.size()] = 0;
  return ret;
}

void deleteCArrayOfOptions(char **options)
{
  int i = 0;
  while (options[i] != 0)
  {
    delete[] options[i++];
  }
  delete[] options;
}

std::ostream& operator<<(std::ostream& out, struct mg_connection *conn)
{
  std::cout << "----------------" << std::endl;
  
  for (int i = 0; i < conn->num_headers; ++i)
    std::cout << conn->http_headers[i].name << " -> " << conn->http_headers[i].value << std::endl;
  
  std::cout << std::string(conn->content, conn->content + conn->content_len) << std::endl;
  
  std::cout << "----------------" << std::endl;
  
  return out;
}

std::pair<std::string, std::string> getUsernamePassword(struct mg_connection *conn)
{
  std::string content(conn->content, conn->content + conn->content_len);
  std::vector<std::string> tokens;
  
  ::boost::split(tokens, content, boost::is_any_of("=&"));
  
  std::string username, password;
  
  for (std::size_t i = 0; i < tokens.size(); ++i)
  {
    if (tokens[i] == "user")
      username = tokens[i + 1];
    else if (tokens[i] == "password")
      password = tokens[i + 1];
    ++i;
  }
  
  return std::make_pair(username, password);
}

std::string getLoginPage(const std::string& document_root)
{
  std::ifstream file((document_root + "/login.html").c_str());
  std::string content {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  file.close();
  return content;
}

std::string getAuthToken(struct mg_connection *conn)
{
  const char* cookies = mg_get_header(conn, "Cookie");
  
  if (!cookies) return "";
 
  std::string cookies_str = cookies;
  std::vector<std::string> tokens;
  
  ::boost::split(
    tokens,
    cookies_str,
    boost::is_any_of(";= "),
    boost::token_compress_on);
  
  auto authtokenIt = std::find(tokens.cbegin(), tokens.cend(), "authtoken");

  return authtokenIt == tokens.cend() ? "" : *(++authtokenIt);
}

std::string getContent(mg_connection *conn)
{
  return std::string(conn->content, conn->content + conn->content_len);
}

std::string getRemoteIp(mg_connection *conn)
{
  return conn->remote_ip;
}

std::string ipToString(unsigned ip)
{
    unsigned char bytes[4];
    bytes[0] = (ip >>  0) & 0xFF;
    bytes[1] = (ip >>  8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    std::string ret;

    ret += std::to_string(bytes[3]);
    ret += '.';
    ret += std::to_string(bytes[2]);
    ret += '.';
    ret += std::to_string(bytes[1]);
    ret += '.';
    ret += std::to_string(bytes[0]);

    return ret;
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


