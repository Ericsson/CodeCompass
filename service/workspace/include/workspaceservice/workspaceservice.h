#ifndef CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
#define CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H

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
  WorkspaceServiceHandler(const std::string& workspace_);

  void getWorkspaces(std::vector<WorkspaceInfo>& _return) override;

private:
  std::string _workspace;
};

} // workspace
} // service
} // cc

#endif // CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
