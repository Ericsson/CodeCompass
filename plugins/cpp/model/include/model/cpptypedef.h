#ifndef CC_MODEL_CPPTYPEDEF_H
#define CC_MODEL_CPPTYPEDEF_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppTypedef : CppTypedEntity
{
  std::string toString() const
  {
    return std::string("CppTypedef")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nqualifiedType = ").append(qualifiedType);
  }
};

typedef std::shared_ptr<CppTypedef> CppTypedefPtr;

#pragma db view object(CppTypedef)
struct CppTypedefCount
{
  #pragma db column("count(" + CppTypedef::id + ")")
  std::size_t count;
};

}
}

#endif
