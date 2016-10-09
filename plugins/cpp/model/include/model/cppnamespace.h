#ifndef CC_MODEL_CPPNAMESPACE_H
#define CC_MODEL_CPPNAMESPACE_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppNamespace : CppEntity
{
  std::string toString() const
  {
    return std::string("CppNamespace")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName);
  }
};

typedef std::shared_ptr<CppNamespace> CppNamespacePtr;

}
}

#endif
