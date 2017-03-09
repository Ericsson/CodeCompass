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

/**
 * Represents a view over a query which results in a single transient unsigned
 * number, such as a "SELECT COUNT(*) FROM something;".
 */
#pragma db view
struct SingleCountView
{
  size_t value;
};

} // model
} // cc

#endif // CC_MODEL_STATISTICS_H
