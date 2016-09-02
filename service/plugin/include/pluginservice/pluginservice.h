#ifndef CC_SERVICE_PLUGIN_PLUGINSERVICE_H
#define CC_SERVICE_PLUGIN_PLUGINSERVICE_H

#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>
#include <PluginService.h>

namespace cc
{ 
namespace service
{
namespace plugin
{

class PluginServiceHandler : virtual public PluginServiceIf
{
public:
  PluginServiceHandler(
    webserver::PluginHandler<cc::webserver::RequestHandler>*);

  void getPlugins(std::vector<std::string>& return_) override;

private:
  webserver::PluginHandler<webserver::RequestHandler>* _pluginHandler;
};

} // plugin
} // service
} // cc

#endif // CC_SERVICE_PLUGIN_PLUGINSERVICE_H
