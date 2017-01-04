#include <boost/program_options.hpp>
#include <util/webserverutil.h>

namespace cc
{
namespace util
{

WorkspaceOptions parseConfigFile(const std::string& filePath_)
{
  //--- Common object ---//

  const std::string prefix = "workspace.";
  const std::size_t prefLen = prefix.length();

  std::map<std::string, std::string WorkspaceOption::*> wsOptAttrs = {
    { "connection",  &WorkspaceOption::connectionString },
    { "description", &WorkspaceOption::description },
    { "datadir",     &WorkspaceOption::datadir }
  };

  //--- Parse config file ---//

  namespace po = boost::program_options;

  WorkspaceOptions result;

  po::options_description emptyopts;
  auto rawOptions = po::parse_config_file<char>(
    filePath_.c_str(), emptyopts, true);

  for (const auto& opt : rawOptions.options)
  {
    //--- Check config validity ---//

    const std::string& key = opt.string_key;
    if (key.find(prefix) != 0)
      throw std::runtime_error("Unknown option in workspace config: " + key);

    std::size_t wsNameEnd = key.find('.', prefLen);
    if (wsNameEnd == std::string::npos)
      throw std::runtime_error(
        "Bad workspace name in workspace config: " + key);

    if (opt.value.size() != 1)
      throw std::runtime_error(
        "Bad value for " + key + " in workspace config");

    //--- Store option ---//

    result[key.substr(prefLen, wsNameEnd - prefLen)]
      .*wsOptAttrs[key.substr(wsNameEnd + 1)] = opt.value[0];
  }

  return result;
}

}
}
