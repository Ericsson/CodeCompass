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
  object(CppFunction) object(CppVariable = Parameters : CppFunction::parameters)
struct CppFunctionParamCount
{
  #pragma db column("count(" + Parameters::id + ")")
  std::size_t count;
};

#pragma db view \
  object(CppFunction) object(CppVariable = Locals : CppFunction::locals)
struct CppFunctionLocalCount
{
  #pragma db column("count(" + Locals::id + ")")
  std::size_t count;
};

}
}

#endif
