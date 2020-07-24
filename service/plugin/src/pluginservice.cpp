#include <pluginservice/pluginservice.h>

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

void PluginServiceHandler::getWebPlugins(std::vector<std::string>& return_)
{
  std::string webrootDir = _configuration["webguiDir"].as<std::string>();

  std::vector<std::string> generatedFiles = getFiles(
    webrootDir + "/scripts/codecompass/generated");

  std::vector<std::string> jsModules = getFiles(
    webrootDir + "/scripts/codecompass/view");

  for (const std::string& file_ : generatedFiles)
    return_.push_back(file_.substr(webrootDir.size() + 1)); // +1 to remove starting slash
  for (const std::string& file_ : jsModules)
    return_.push_back(file_.substr(webrootDir.size() + 1)); // +1 to remove starting slash
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
