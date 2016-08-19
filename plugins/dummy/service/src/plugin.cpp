#include <webserver/pluginhelper.h>

#include <service/dummyservice.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  namespace po = boost::program_options;

  po::options_description description("Dummy Plugin");

  description.add_options()
    ("dummy-result", po::value<std::string>()->default_value("Dummy result"),
      "This value will be returned by the dummy service.");

  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& configuration,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler)
{
  cc::webserver::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Dummy, dummy),
    "DummyService");
}

}
