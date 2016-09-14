#include <plugin/pluginhelper.h>

#include "workspaceservice.h"

boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Workspace Plugin");

  return description;
}

void registerPlugin(const boost::program_options::variables_map& config_,
  cc::plugin::PluginHandler<cc::mongoose::RequestHandler>*
  pluginHandler_)
{
  using namespace cc::mongoose;
  using namespace cc::service::workspace;

  const WorkspaceOptions& workspaces =
    config_[WORKSPACE_OPTION_NAME].as<WorkspaceOptions>();

  std::shared_ptr<RequestHandler> handler(
    new ThriftHandler<WorkspaceServiceProcessor>(
      new WorkspaceServiceHandler(workspaces), "*"));

  pluginHandler_->registerImplementation(
    "WorkspaceService", handler, RequestHandler::version);
}


