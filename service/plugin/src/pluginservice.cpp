#include <pluginservice/pluginservice.h>

namespace cc
{ 
namespace service
{
namespace plugin
{

PluginServiceHandler::PluginServiceHandler(
  webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
    : _pluginHandler(pluginHandler_)
{
}

void PluginServiceHandler::getPlugins(std::vector<std::string>& return_)
{
  for (const auto& it : _pluginHandler->getImplementationMap())
    return_.push_back(it.first);
}

}
}
}
