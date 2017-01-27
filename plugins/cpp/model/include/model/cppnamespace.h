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
    std::string ret("CppNamespace");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName);

    if (!tags.empty())
    {
      ret.append("\ntags =");
      for (const Tag& tag : tags)
        ret.append(' ' + tagToString(tag));
    }

    return ret;
  }
};

typedef std::shared_ptr<CppNamespace> CppNamespacePtr;

}
}

#endif
