#include <webserver/pluginhelper.h>

#include <service/gitservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    namespace po = boost::program_options;
    po::options_description description("Git Plugin");
    return description;
  }

  void registerPlugin(
    const boost::program_options::variables_map& configuration,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler)
  {
    cc::webserver::registerPluginSimple(
      configuration,
      pluginHandler,
      CODECOMPASS_SERVICE_FACTORY_WITH_CFG(Git, git),
      "GitService");
  }
}
#pragma clang diagnostic pop
