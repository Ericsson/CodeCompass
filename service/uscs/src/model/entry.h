#ifndef __CC_USCS_MODEL_ENTRY_H__
#define __CC_USCS_MODEL_ENTRY_H__

#include <string>
#include <map>
#include <ctime>

#include "server.h"

namespace cc
{
namespace service
{
namespace stat
{

#pragma db object
struct UsageEntry
{
  #pragma db id auto
  unsigned long long id;

  #pragma db value_not_null
  std::shared_ptr<Server> server;

  #pragma db value_not_null unordered
  std::map<std::string, std::string> props;

  #pragma db not_null
  std::time_t timestamp = 0;
};

} // stat
} // service
} // cc

#endif // __CC_USCS_MODEL_ENTRY_H__

