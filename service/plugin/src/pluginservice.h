#ifndef __PLUGIN_SERVICE_H__
#define __PLUGIN_SERVICE_H__

#include <mongoose/plugin.h>
#include <plugin-api/PluginService.h>

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

}
}
}

#endif