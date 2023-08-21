#include <webserver/pluginhelper.h>

#include <service/helmservice.h>

/* These two methods are used by the plugin manager to allow dynamic loading
  of CodeCompass Service plugins. Clang (>= version 6.0) gives a warning that
  these C-linkage specified methods return types that are not proper from a
  C code.

  These codes are NOT to be called from any C code. The C linkage is used to
  turn off the name mangling so that the dynamic loader can easily find the
  symbol table needed to set the plugin up.
*/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    namespace po = boost::program_options;

    po::options_description description("YAML Plugin");

    description.add_options()
      ("dummy-result", po::value<std::string>()->default_value("Dummy result"),
        "This value will be returned by the dummy service.");

    return description;
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    cc::webserver::registerPluginSimple(
      context_,
      pluginHandler_,
      CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Helm, language),
      "YamlService");
  }
}
#pragma clang diagnostic pop
