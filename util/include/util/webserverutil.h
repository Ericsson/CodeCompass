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

WorkspaceOptions parseConfigFile(const std::string& filePath_);

}
}

#endif
