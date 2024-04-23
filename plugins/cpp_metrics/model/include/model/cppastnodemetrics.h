#ifndef CC_MODEL_CPPASTNODEMETRICS_H
#define CC_MODEL_CPPASTNODEMETRICS_H

#include <model/cppastnode.h>
#include <model/cppentity.h>
#include <model/cpprecord.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppAstNodeMetrics
{
  enum Type
  {
    PARAMETER_COUNT = 1,
    MCCABE = 2,
    LACK_OF_COHESION = 3,
    LACK_OF_COHESION_HS = 4,
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  CppAstNodeId astNodeId;

  #pragma db not_null
  Type type;

  #pragma db null
  double value;
};

#pragma db view \
  object(CppRecord) \
  object(CppAstNodeMetrics : \
    CppRecord::astNodeId == CppAstNodeMetrics::astNodeId)
struct CppRecordMetricsView
{
  #pragma db column(CppAstNodeMetrics::value)
  double value;
};

#pragma db view \
  object(CppAstNode) \
  object(File = LocFile : CppAstNode::location.file) \
  object(CppAstNodeMetrics : CppAstNode::id == CppAstNodeMetrics::astNodeId)
struct CppAstNodeMetricsForPathView
{
  #pragma db column(CppAstNode::id)
  CppAstNodeId astNodeId;

  #pragma db column(CppAstNodeMetrics::type)
  CppAstNodeMetrics::Type type;

  #pragma db column(CppAstNodeMetrics::value)
  double value;
};

} //model
} //cc

#endif //CC_MODEL_CPPASTNODEMETRICS_H
