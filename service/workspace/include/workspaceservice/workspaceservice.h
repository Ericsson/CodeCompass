#ifndef CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
#define CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H

#include <util/webserverutil.h>
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
  typedef cc::util::WorkspaceOptions WorkspaceOptions;

  WorkspaceServiceHandler(const util::WorkspaceOptions& workspaces_);
  void getWorkspaces(std::vector<WorkspaceInfo>& _return) override;

private:
  util::WorkspaceOptions _workspaces;
};

} // workspace
} // service
} // cc

#endif // CC_SERVICE_WORKSPACE_WORKSPACESERVICE_H
