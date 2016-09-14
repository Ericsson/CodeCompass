#ifndef __CC_USCS_MODEL_SERVER_H__
#define __CC_USCS_MODEL_SERVER_H__

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <odb/vector.hxx>

namespace cc
{
namespace service
{
namespace stat
{

#pragma db object
struct ServerAddress
{
  #pragma db id auto
  unsigned long long id;

  #pragma db not_null unique index
  std::string address;
};

#pragma db object
struct ServerPort
{
  #pragma db id auto
  unsigned long long id;

  #pragma db not_null unique index
  std::string port;
};

#pragma db object
struct Server
{
  #pragma db id auto
  unsigned long long id;

  #pragma db value_not_null
  odb::vector<std::shared_ptr<ServerAddress>> addresses;

  #pragma db value_not_null
  odb::vector<std::shared_ptr<ServerPort>> ports;

  #pragma db value_not_null unordered
  std::map<std::string, std::string> props;

  #pragma db not_null
  std::time_t lastContact = 0;
};

#pragma db view object(Server) \
  object(ServerAddress : Server::addresses) \
  object(ServerPort : Server::ports)
struct ServerByInfo
{
  #pragma db column(Server::id)
  unsigned long long id;
};

} // stat
} // service
} // cc

#endif // __CC_USCS_MODEL_SERVER_H__

