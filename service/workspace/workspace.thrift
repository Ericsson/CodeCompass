namespace cpp cc.service.workspace
namespace java cc.service.workspace

/**
 * Workspace description.
 */
struct WorkspaceInfo
{
  1:string id,
  2:string description
}

/**
 * The workspace service.
 */
service WorkspaceService
{
  /**
   * Returns a list of active workspaces
   */
  list<WorkspaceInfo> getWorkspaces()
}
