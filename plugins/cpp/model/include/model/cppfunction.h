#ifndef CC_MODEL_CPPFUNCTION_H
#define CC_MODEL_CPPFUNCTION_H

#include <vector>

#include "cppentity.h"
#include "cppvariable.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppFunction : CppTypedEntity
{
  #pragma db on_delete(cascade)
  std::vector<odb::lazy_shared_ptr<CppVariable>> parameters;
  #pragma db on_delete(cascade)
  std::vector<odb::lazy_shared_ptr<CppVariable>> locals;

  unsigned int mccabe;
  unsigned int bumpiness;
  unsigned int statementCount;

  std::string toString() const
  {
    return std::string("CppFunction")
      .append("\nid = ").append(std::to_string(id))
      .append("\nentityHash = ").append(std::to_string(entityHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nqualifiedType = ").append(qualifiedType);
  }
};

typedef std::shared_ptr<CppFunction> CppFunctionPtr;

#pragma db view \
  object(CppFunction) object(CppVariable = Parameters inner : CppFunction::parameters)
struct CppFunctionParamCount
{
  #pragma db column("count(" + Parameters::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppFunction) \
  object(CppVariable = Parameters inner : CppFunction::parameters) \
  object(CppAstNode : CppFunction::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file) \
  query((?) + "GROUP BY" + cc::model::CppEntity::astNodeId + "," + cc::model::File::path)
struct CppFunctionParamCountWithId
{
  #pragma db column(CppEntity::astNodeId)
  CppAstNodeId id;

  #pragma db column("count(" + Parameters::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppFunction) object(CppVariable = Locals inner : CppFunction::locals)
struct CppFunctionLocalCount
{
  #pragma db column("count(" + Locals::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppFunction) \
  object(CppAstNode : CppFunction::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file)
struct CppFunctionMcCabe
{
  #pragma db column(CppEntity::astNodeId)
  CppAstNodeId astNodeId;

  #pragma db column(CppFunction::mccabe)
  unsigned int mccabe;
};

#pragma db view \
  object(CppFunction) \
  object(CppAstNode : CppFunction::astNodeId == CppAstNode::id) \
  object(File : CppAstNode::location.file)
struct CppFunctionBumpyRoad
{
  #pragma db column(CppEntity::astNodeId)
  CppAstNodeId astNodeId;

  #pragma db column(CppFunction::bumpiness)
  unsigned int bumpiness;

  #pragma db column(CppFunction::statementCount)
  unsigned int statementCount;
};

#pragma db view \
  object(CppFunction) \
  object(CppVariable = Parameters inner : CppFunction::parameters)
struct CppFunctionParamTypeView
{
  #pragma db column(CppFunction::astNodeId)
  CppAstNodeId astNodeId;

  #pragma db column(Parameters::typeHash)
  std::uint64_t paramTypeHash;
};

#pragma db view \
  object(CppFunction) \
  object(CppVariable = Locals inner : CppFunction::locals)
struct CppFunctionLocalTypeView
{
#pragma db column(CppFunction::astNodeId)
  CppAstNodeId astNodeId;

#pragma db column(Locals::typeHash)
  std::uint64_t paramTypeHash;
};
}
}

#endif
