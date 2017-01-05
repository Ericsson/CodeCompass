#include <plugin/pluginhelper.h>

#include "pluginservice.h"

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Plugin Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& config_,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler_)
{
  using namespace cc::mongoose;
  using namespace cc::service::plugin;

  std::shared_ptr<RequestHandler> handler(
    new ThriftHandler<PluginServiceProcessor>(
      new PluginServiceHandler(pluginHandler_), "*"));

  pluginHandler_->registerImplementation(
    "PluginService", handler, RequestHandler::version);
}
