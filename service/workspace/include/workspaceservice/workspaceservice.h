#ifndef CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
#define CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H

#include <mongoose/plugin.h>

#include <WorkspaceService.h>

namespace cc
{ 
namespace service
{
namespace workspace 
{

class WorkspaceServiceHandler : virtual public WorkspaceServiceIf
{
public:
  typedef cc::mongoose::WorkspaceOptions WorkspaceOptions;

public:
  WorkspaceServiceHandler(const WorkspaceOptions& workspaces_);

public:
  void getWorkspaces(std::vector<WorkspaceInfo>& _return) override;

private:
  WorkspaceOptions  _workspaces;
};

} // workspace
} // service
} // cc

#endif // CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
