#include <boost/filesystem.hpp>
#include <workspaceservice/workspaceservice.h>

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

void WorkspaceServiceHandler::getWorkspaces(std::vector<WorkspaceInfo>& _return)
{
  namespace fs = boost::filesystem;

  for (fs::directory_iterator it(_workspace);
    it != fs::directory_iterator();
    ++it)
  {
    std::string filename = it->path().filename().native();

    WorkspaceInfo info;
    info.__set_id(filename);
    info.__set_description(filename);

    _return.push_back(std::move(info));
  }
}

} // workspace
} // service
} // cc
