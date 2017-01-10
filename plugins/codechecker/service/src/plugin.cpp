#include <webserver/pluginhelper.h>

#include <service/codecheckerproxy.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  namespace po = boost::program_options;

  boost::program_options::options_description description("CodeChecker Plugin");

  description.add_options()
    ("codechecker-url", po::value<std::string>()->default_value(
        "http://localhost:8001/CodeCheckerService"),
      "A connection string of the CodeChecker service "
      "http://localhost:8001/CodeCheckerService.")
    ("codechecker-runname", po::value<std::string>()->default_value("codechecker"),
      "Name of the analysis.");

  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& configuration,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler)
{
  cc::webserver::registerPluginSimple(
    configuration,
    pluginHandler,
    CODECOMPASS_SERVICE_FACTORY_WITH_CFG_NOPOSTFIXSERVICE(codeCheckerDBAccess, CodeCheckerProxy, codechecker),
    "CodeCheckerService");
}

}
