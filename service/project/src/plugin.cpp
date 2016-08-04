#include <plugin/pluginhelper.h>

#include <projectservice/projectservice.h>

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Core Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  cc::plugin::registerPluginSimple(
      configuration,
      pluginHandler,
      CODECOMPASS_SIMPLE_SERVICE_FACTORY(Project, core),
      "ProjectService");
}


