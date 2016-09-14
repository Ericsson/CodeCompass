#ifndef CC_MODEL_STATISTICS_H
#define CC_MODEL_STATISTICS_H

#include <memory>
#include <string>

#include <odb/core.hxx>

namespace cc
{
namespace model
{

struct Statistics;
typedef std::shared_ptr<Statistics> StatisticsPtr;
typedef int StatisticsId;

#pragma db object
struct Statistics
{
  #pragma db id auto
  unsigned long id;

  std::string group;
  std::string key;
  int         value;
};

} // model
} // cc

#endif // CC_MODEL_STATISTICS_H
