#include <pluginservice/pluginservice.h>

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
  std::vector<std::string> dirs =
  {
    webrootDir + "/scripts/codecompass/generated",
    webrootDir + "/scripts/codecompass/view"
  };

  for (const std::string& dir : dirs)
  {
    boost::filesystem::recursive_directory_iterator it(dir), end;
    while (it != end)
    {
      if (!boost::filesystem::is_directory(it->path()))
        return_.push_back(it->path().string().substr(webrootDir.size()));

      ++it;
    }
  }
}

}
}
}
