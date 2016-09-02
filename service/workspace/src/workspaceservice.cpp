#include <workspaceservice/workspaceservice.h>

namespace cc
{ 
namespace service
{
namespace workspace 
{

WorkspaceServiceHandler::WorkspaceServiceHandler(
  const util::WorkspaceOptions& workspaces_) : _workspaces(workspaces_)
{
}

void WorkspaceServiceHandler::getWorkspaces(std::vector<WorkspaceInfo>& _return)
{
  for (const auto& ws : _workspaces)
  {
    WorkspaceInfo info;

    info.id = ws.first;
    info.description = ws.second.description;

    _return.push_back(std::move(info));
  }
}

} // workspace
} // service
} // cc
