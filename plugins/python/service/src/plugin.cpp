#include <webserver/pluginhelper.h>

#include <service/pythonservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    namespace po = boost::program_options;

    po::options_description description("Python Plugin");

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
      CODECOMPASS_LANGUAGE_SERVICE_FACTORY_WITH_CFG(Python),
      "PythonService");
  }
}
#pragma clang diagnostic pop
