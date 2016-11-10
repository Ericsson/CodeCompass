#include <webserver/pluginhelper.h>

#include <metricsservice/metricsservice.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Metrics Plugin");

  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& configuration,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler)
{
  cc::webserver::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Metrics, metrics),
    "MetricsService");
}

}
