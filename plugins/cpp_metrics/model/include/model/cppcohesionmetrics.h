#ifndef CC_MODEL_CPPCOHESIONMETRICS_H
#define CC_MODEL_CPPCOHESIONMETRICS_H

#include <model/cppentity.h>
#include <model/cpprecord.h>
#include <model/cppastnode.h>

namespace cc
{
namespace model
{
#pragma db view \
  object(CppRecord) \
  object(CppAstNode : CppRecord::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file)
struct CohesionCppRecordView
{
  #pragma db column(CppEntity::entityHash)
  std::size_t entityHash;

  #pragma db column(CppEntity::qualifiedName)
  std::string qualifiedName;

  #pragma db column(CppEntity::astNodeId)
  CppAstNodeId astNodeId;
};

#pragma db view \
  object(CppRecord) \
  object(CppAstNode : CppRecord::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file)
struct CohesionCppRecord_Count
{
  #pragma db column("count(" + CppEntity::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppMemberType) \
  object(CppAstNode : CppMemberType::memberAstNode) \
  query(CppMemberType::kind == cc::model::CppMemberType::Kind::Field && (?))
struct CohesionCppFieldView
{
  #pragma db column(CppAstNode::entityHash)
  std::size_t entityHash;
};

#pragma db view \
  object(CppMemberType) \
  object(CppAstNode : CppMemberType::memberAstNode) \
  object(File : CppAstNode::location.file) \
  query(CppMemberType::kind == cc::model::CppMemberType::Kind::Method && (?))
struct CohesionCppMethodView
{
  typedef cc::model::Position::PosType PosType;

  #pragma db column(CppAstNode::location.range.start.line)
  PosType startLine;
  #pragma db column(CppAstNode::location.range.start.column)
  PosType startColumn;
  #pragma db column(CppAstNode::location.range.end.line)
  PosType endLine;
  #pragma db column(CppAstNode::location.range.end.column)
  PosType endColumn;

  #pragma db column(File::path)
  std::string filePath;
};

#pragma db view \
  object(CppAstNode) \
  object(File : CppAstNode::location.file) \
  query((CppAstNode::astType == cc::model::CppAstNode::AstType::Read \
      || CppAstNode::astType == cc::model::CppAstNode::AstType::Write) && (?))
struct CohesionCppAstNodeView
{
  #pragma db column(CppAstNode::entityHash)
  std::uint64_t entityHash;
};

} //model
} //cc

#endif //CC_MODEL_CPPCOHESIONMETRICS_H
