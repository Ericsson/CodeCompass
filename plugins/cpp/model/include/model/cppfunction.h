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
  std::vector<odb::lazy_shared_ptr<CppVariable>> parameters;
  std::vector<odb::lazy_shared_ptr<CppVariable>> locals;

  std::string toString() const
  {
    return std::string("CppFunction")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nqualifiedType = ").append(qualifiedType);
  }
};

typedef std::shared_ptr<CppFunction> CppFunctionPtr;

}
}

#endif
