#include <plugin/pluginhelper.h>

#include <service/dummyservice.h>

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Dummy Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  cc::plugin::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Dummy, dummy),
    "DummyService");
}



