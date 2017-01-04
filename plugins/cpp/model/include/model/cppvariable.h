#ifndef CC_MODEL_CPPVARIABLE_H
#define CC_MODEL_CPPVARIABLE_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppVariable : CppTypedEntity
{
  bool isGlobal = false;

  std::string toString() const
  {
    return std::string("CppVariable")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nqualifiedType = ").append(qualifiedType)
      .append("\nisGlobal = ").append(std::to_string(isGlobal));
  }
};

typedef std::shared_ptr<CppVariable> CppVariablePtr;

}
}

#endif
