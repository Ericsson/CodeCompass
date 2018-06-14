#include <webserver/pluginhelper.h>

#include <metricsservice/metricsservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    return boost::program_options::options_description("Metrics Plugin");
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    cc::webserver::registerPluginSimple(
      context_,
      pluginHandler_,
      CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Metrics, metrics),
      "MetricsService");
  }
}
#pragma clang diagnostic pop
