#ifndef CC_SERVICE_PLUGIN_PLUGINSERVICE_H
#define CC_SERVICE_PLUGIN_PLUGINSERVICE_H

#include <mongoose/plugin.h>

#include <PluginService.h>

#include <vector>

namespace cc
{ 
namespace service
{
namespace plugin
{

class PluginServiceHandler : virtual public PluginServiceIf
{
public:
  PluginServiceHandler(cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*);
  void getPlugins(std::vector<std::string> & _return);
  
private:
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>* _pluginHandler;
};

} // plugin
} // service
} // cc

#endif // CC_SERVICE_PLUGIN_PLUGINSERVICE_H
