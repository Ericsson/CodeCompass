#include <algorithm>

#include <pluginservice/pluginservice.h>

namespace cc
{ 
namespace service
{
namespace plugin
{

PluginServiceHandler::PluginServiceHandler(
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>* pluginHandler_)
    : _pluginHandler(pluginHandler_)
{
}

void PluginServiceHandler::getPlugins(std::vector<std::string>& _return)
{
  for (const auto& it : _pluginHandler->getImplementationMap())
    _return.push_back(it.first);
}

}
}
}
