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
  /**
   * This function defines an ordering between WorkspaceInfo objects by name.
   */
  static bool workspaceInfoOrder(const WorkspaceInfo& left, const WorkspaceInfo& right);

  std::string _workspace;
};

} // workspace
} // service
} // cc

#endif // CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
