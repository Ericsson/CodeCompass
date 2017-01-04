#include <webserver/pluginhelper.h>

#include <service/cppservice.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("C++ Plugin");
  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& configuration,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler)
{
  cc::webserver::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_LANGUAGE_SERVICE_FACTORY_WITH_CFG(Cpp),
    "CppService");
}

}
