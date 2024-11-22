#include <boost/filesystem.hpp>
#include <workspaceservice/workspaceservice.h>
#include <algorithm>

namespace cc
{ 
namespace service
{
namespace workspace 
{

WorkspaceServiceHandler::WorkspaceServiceHandler(const std::string& workspace_)
  : _workspace(workspace_)
{
}

void WorkspaceServiceHandler::getWorkspaces(std::vector<WorkspaceInfo>& return_)
{
  namespace fs = boost::filesystem;

  for (fs::directory_iterator it(_workspace);
    it != fs::directory_iterator();
    ++it)
  {
    if (!fs::is_directory(it->path()))
      // Ignore plain files in the workspace directory - projects are always
      // directories.
      continue;
    if (!fs::is_regular_file(fs::path{it->path()}.append("project_info.json")))
      // Ignore directories that do not have a project information for them.
      // (cf. webserver/pluginhelper.h)
      continue;

    std::string filename = it->path().filename().native();
    WorkspaceInfo info;
    info.id = filename;
    info.description = filename;

    return_.push_back(std::move(info));
  }

  std::sort(return_.begin(), return_.end(), workspaceInfoOrder);
}

bool WorkspaceServiceHandler::workspaceInfoOrder(
  const WorkspaceInfo& left_,
  const WorkspaceInfo& right_)
{
  return left_.id < right_.id;
}

} // workspace
} // service
} // cc
