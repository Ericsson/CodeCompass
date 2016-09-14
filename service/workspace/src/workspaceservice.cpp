#include "workspaceservice.h"

namespace cc
{ 
namespace service
{
namespace workspace 
{

WorkspaceServiceHandler::WorkspaceServiceHandler(
  const WorkspaceOptions& workspaces_) :
  _workspaces(workspaces_)
{
}

void WorkspaceServiceHandler::getWorkspaces(
  std::vector<WorkspaceInfo>& _return)
{
  for (const auto& ws : _workspaces)
  {
    WorkspaceInfo info;
    info.id = ws.workspaceId;
    info.description = ws.description;

    _return.emplace_back(std::move(info));
  }
}

} // workspace
} // service
} // cc
