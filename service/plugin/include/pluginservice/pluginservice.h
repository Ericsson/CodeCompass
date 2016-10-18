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
    webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_,
    const boost::program_options::variables_map& configuration_);

  void getPlugins(std::vector<std::string>& return_) override;

  void getWebPlugins(std::vector<std::string> & _return) override;
private:
  webserver::PluginHandler<webserver::RequestHandler>* _pluginHandler;
  const boost::program_options::variables_map& _configuration;
};

} // plugin
} // service
} // cc

#endif // CC_SERVICE_PLUGIN_PLUGINSERVICE_H
