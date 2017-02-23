#ifndef CC_MODEL_CPPMACRO_H
#define CC_MODEL_CPPMACRO_H

#include "cppentity.h"
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct CppMacro : CppEntity
{
  std::string toString() const
  {
    std::string ret("CppMacro");

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

typedef std::shared_ptr<CppMacro> CppMacroPtr;

} // model
} // cc

#endif // CC_MODEL_CPPMACRO_H
