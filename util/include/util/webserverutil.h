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
 *
 * [workspace.project2]
 * connection = sqlite:database=database.sqlite
 * description = Project2
 * ~~~
 */
WorkspaceOptions parseConfigFile(const std::string& filePath_);

}
}

#endif
