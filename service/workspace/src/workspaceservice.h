#ifndef __WORKSPACE_SERVICE_H__
#define __WORKSPACE_SERVICE_H__

#include <mongoose/plugin.h>
#include <workspace-api/WorkspaceService.h>

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

#endif // __WORKSPACE_SERVICE_H__
