#ifndef CC_MODEL_CPPASTNODEMETRICS_H
#define CC_MODEL_CPPASTNODEMETRICS_H

#include <model/cppastnode.h>
#include <model/cppentity.h>
#include <model/cpprecord.h>
#include <model/position.h>

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
    MCCABE_FUNCTION = 2,
    MCCABE_TYPE = 3,
    BUMPY_ROAD = 4,
    LACK_OF_COHESION = 5,
    LACK_OF_COHESION_HS = 6,
    EFFERENT_TYPE = 7,
    AFFERENT_TYPE = 8
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
  object(CppAstNodeMetrics) \
  object(CppAstNode : CppAstNodeMetrics::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file)
struct CppAstNodeMetricsFileView
{
  #pragma db column(CppAstNode::id)
  CppAstNodeId astNodeId;

  #pragma db column(File::id)
  FileId fileId;
};
  
#pragma db view \
  object(CppAstNodeMetrics) \
  object(CppAstNode : CppAstNodeMetrics::astNodeId == CppAstNode::id) \
  object(File = LocFile : CppAstNode::location.file)
struct CppAstNodeMetricsForPathView
{
  #pragma db column(CppAstNodeMetrics::astNodeId)
  CppAstNodeId astNodeId;

  #pragma db column(LocFile::path)
  std::string path;

  #pragma db column(CppAstNodeMetrics::type)
  CppAstNodeMetrics::Type type;

  #pragma db column(CppAstNodeMetrics::value)
  double value;
};

#pragma db view \
  object(CppAstNodeMetrics) \
  object(CppAstNode : CppAstNodeMetrics::astNodeId == CppAstNode::id) \
  object(File = LocFile : CppAstNode::location.file)
struct CppAstNodeMetricsAndDataForPathView
{
  typedef cc::model::Position::PosType PosType;

  #pragma db column(CppAstNodeMetrics::astNodeId)
  CppAstNodeId astNodeId;

  #pragma db column(LocFile::path)
  std::string path;

  #pragma db column(CppAstNode::location.range.start.line)
  PosType startLine;

  #pragma db column(CppAstNode::location.range.end.line)
  PosType endLine;

  #pragma db column(CppAstNode::astValue)
  std::string astValue;

  #pragma db column(CppAstNode::symbolType)
  CppAstNode::SymbolType symbolType;

  #pragma db column(CppAstNode::astType)
  CppAstNode::AstType astType;

  #pragma db column(CppAstNodeMetrics::type)
  CppAstNodeMetrics::Type type;

  #pragma db column(CppAstNodeMetrics::value)
  double value;
};

} //model
} //cc

#endif //CC_MODEL_CPPASTNODEMETRICS_H
