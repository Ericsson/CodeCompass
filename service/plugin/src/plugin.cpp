#include <webserver/requesthandler.h>
#include <webserver/thrifthandler.h>
#include <pluginservice/pluginservice.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Plugin Plugin");

  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& config_,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
{
  std::shared_ptr<cc::webserver::RequestHandler> handler(
    new cc::webserver::ThriftHandler<cc::service::plugin::PluginServiceProcessor>(
      new cc::service::plugin::PluginServiceHandler(pluginHandler_, config_), "*"));

  pluginHandler_->registerImplementation("PluginService", handler);
}

}
