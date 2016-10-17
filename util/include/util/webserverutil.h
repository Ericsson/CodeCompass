#ifndef CC_WEBSERVER_UTIL_H
#define CC_WEBSERVER_UTIL_H

#include <string>
#include <map>
#include <stdexcept>

namespace cc
{
namespace util
{

class ServiceNotAvailException : public std::runtime_error
{
public:
  explicit ServiceNotAvailException(const std::string& msg_)
    : std::runtime_error(msg_)
  {
  }
};

struct WorkspaceOption
{
  std::string connectionString;
  std::string description;
  std::string datadir;
  std::string searchdir;
};

using WorkspaceOptions = std::map<std::string, WorkspaceOption>;

/**
 * This function parses a workspace config file. This file has to be in INI-like
 * format:
 *
 * ~~~
 * [workspace.project1]
 * connection = sqlite:database=database.sqlite
 * description = Project1
 * datadir = /home/eusername/workdir/project1/data
 * searchdir = /home/eusername/workdir/project1/data/search
 *
 * [workspace.project2]
 * connection = sqlite:database=database.sqlite
 * description = Project2
 * datadir = /home/eusername/workdir/project2/data
 * searchdir = /home/eusername/workdir/project2/data/search
 * ~~~
 */
WorkspaceOptions parseConfigFile(const std::string& filePath_);

}
}

#endif
