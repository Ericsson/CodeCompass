#ifndef CC_MODEL_CPPASTNODEMETRICS_H
#define CC_MODEL_CPPASTNODEMETRICS_H

//#include <../../../../plugins/cpp/model/include/model/cppastnode.h>
#include <model/cppastnode.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppAstNodeMetrics
{
  enum Type
  {
    PARAMETER_COUNT
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  CppAstNodeId astNodeId;

  #pragma db not_null
  Type type;

  #pragma db not_null
  unsigned value;
};

} //model
} //cc

#endif //CC_MODEL_CPPASTNODEMETRICS_H
