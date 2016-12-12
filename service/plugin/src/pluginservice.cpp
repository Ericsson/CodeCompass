#include <pluginservice/pluginservice.h>

namespace
{

std::vector<std::string> getFiles(std::string dir_)
{
  std::vector<std::string> res;

  for(boost::filesystem::directory_iterator it(dir_), end; it != end; ++it)
    if(!boost::filesystem::is_directory(it->path()))
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
  const boost::program_options::variables_map& configuration_)
    : _pluginHandler(pluginHandler_), _configuration(configuration_)
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

  for(std::string& file_ : generatedFiles)
    return_.push_back(file_.substr(webrootDir.size()));
  for(std::string& file_ : jsModules)
    return_.push_back(file_.substr(webrootDir.size()));
}

void PluginServiceHandler::getWebStylePlugins(std::vector<std::string>& return_)
{
  std::string webrootDir = _configuration["webguiDir"].as<std::string>();

  std::vector<std::string> cssFiles = getFiles(
      webrootDir + "/style");

  for(std::string& file_ : cssFiles)
    return_.push_back(file_.substr(webrootDir.size()));
}

}
}
}
