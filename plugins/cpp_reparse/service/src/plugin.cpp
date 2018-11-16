#include <webserver/pluginhelper.h>

#include <service/cppreparseservice.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    namespace po = boost::program_options;

    boost::program_options::options_description description(
      "C++ Reparse Plugin");

    description.add_options()
      ("disable-cpp-reparse", po::value<bool>()->default_value(false),
       "Turn off the reparse capabilities on the started server, even though "
       "the plugin is loaded.");

    description.add_options()
      ("ast-cache-limit", po::value<size_t>()->default_value(10),
       "The maximum number of reparsed syntax trees that should be cached in "
       "memory.");

    return description;
  }

  void registerPlugin(
    const cc::webserver::ServerContext& context_,
    cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
  {
    cc::webserver::registerPluginSimple(
      context_,
      pluginHandler_,
      CODECOMPASS_SERVICE_FACTORY_WITH_CFG(CppReparse, language),
      "CppReparseService");
  }
}
#pragma clang diagnostic pop
