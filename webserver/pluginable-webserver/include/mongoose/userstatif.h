#ifndef __CC_PLUGINABLE_WEBSERVER_USER_STAT_IF_H__
#define __CC_PLUGINABLE_WEBSERVER_USER_STAT_IF_H__

#include <memory>

struct mg_connection;

namespace cc
{
namespace mongoose
{

class UserStatIf
{
public:
  virtual ~UserStatIf() = default;

  virtual void logMethodCall(
    mg_connection* conn_,
    const std::string workspace_,
    const std::string handler_,
    const std::string method_) = 0;
};

using UserStatPtr = std::shared_ptr<UserStatIf>;

}
}

#endif // __CC_PLUGINABLE_WEBSERVER_USER_STAT_IF_H__
