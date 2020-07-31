#include <pluginservice/pluginservice.h>
#include <boost/regex.hpp>

namespace
{

std::vector<std::string> getFiles(const std::string& dir_)
{
  std::vector<std::string> res;

  for (boost::filesystem::directory_iterator it(dir_), end; it != end; ++it)
    if (!boost::filesystem::is_directory(it->path()))
      res.push_back(it->path().string());

  return res;
}

}

namespace cc
{ 
namespace service
{
namespace plugin
{

PluginServiceHandler::PluginServiceHandler(
  webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_,
  const cc::webserver::ServerContext& context_)
    : _pluginHandler(pluginHandler_),
      _configuration(context_.options)
{
}

void PluginServiceHandler::getPlugins(std::vector<std::string>& return_)
{
  for (const auto& it : _pluginHandler->getImplementationMap())
    return_.push_back(it.first);
}

void PluginServiceHandler::getThriftPlugins(std::vector<std::string>& return_)
{
  std::string webrootDir = _configuration["webguiDir"].as<std::string>();

  std::vector<std::string> generatedFiles = getFiles(
    webrootDir + "/scripts/codecompass/generated");

  for (const std::string& file_ : generatedFiles)
    return_.push_back(file_.substr(webrootDir.size() + 1)); // +1 to remove starting slash
}

void PluginServiceHandler::getWebPlugins(std::vector<std::string>& return_)
{
  std::string webrootDir = _configuration["webguiDir"].as<std::string>();

  std::vector<std::string> jsModules = getFiles(
    webrootDir + "/scripts/release/codecompass/view");

  /*
   * Pattern to match only *.js files which does not contain the '.js.' substring, to exclude:
   * *.js.uncompressed.js
   * *.js.consoleStripped.js
   * *.js.map
   */
  static const boost::regex pattern("^((?!\\.js\\.).)*\\.js$");
  for (const std::string& file_ : jsModules)
  {
    if (boost::regex_match(file_, pattern))
      return_.push_back(file_.substr(webrootDir.size() + 1)); // +1 to remove starting slash
  }
}

void PluginServiceHandler::getWebStylePlugins(std::vector<std::string>& return_)
{
  std::string webrootDir = _configuration["webguiDir"].as<std::string>();

  std::vector<std::string> cssFiles = getFiles(webrootDir + "/style");

  for (const std::string& file_ : cssFiles)
    return_.push_back(file_.substr(webrootDir.size() + 1)); // +1 to remove starting slash
}

}
}
}
