#include <plugin/pluginhelper.h>

#include <pythonservice/pythonservice.h>

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Python Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& configuration,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler)
{
  using namespace cc::mongoose;
  
  auto factory = [](
    std::shared_ptr<odb::database> db_,
    const boost::program_options::variables_map& cfg_) {
    return new ThriftHandler<cc::service::language::LanguageServiceProcessor>(
      new cc::service::language::python::PythonServiceHandler(db_),
      cfg_["workspaceId"].as<std::string>());
  };

  cc::plugin::registerPluginSimple(
    configuration,
    pluginHandler,
    factory,
    "PythonService");
}


