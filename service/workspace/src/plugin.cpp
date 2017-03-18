#include <boost/program_options.hpp>
#include <webserver/pluginhandler.h>
#include <webserver/requesthandler.h>
#include <webserver/thrifthandler.h>
#include <workspaceservice/workspaceservice.h>

extern "C"
{

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Workspace Plugin");

  return description;
}

void registerPlugin(
  const boost::program_options::variables_map& vm_,
  cc::webserver::PluginHandler<cc::webserver::RequestHandler>* pluginHandler_)
{
  std::shared_ptr<cc::webserver::RequestHandler> handler(
    new cc::webserver::ThriftHandler<cc::service::workspace::WorkspaceServiceProcessor>(
      new cc::service::workspace::WorkspaceServiceHandler(
        vm_["workspace"].as<std::string>())));

  pluginHandler_->registerImplementation("WorkspaceService", handler);
}

}
