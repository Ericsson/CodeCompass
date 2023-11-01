#ifndef CC_MODEL_CPPNAMESPACEALIAS_H
#define CC_MODEL_CPPNAMESPACEALIAS_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppNamespaceAlias : CppEntity
{
  #pragma db unique
  CppAstNodeId aliasedNamespaceNodeId;

  CppAstNodePtr aliasedNamespace;

  std::string toString() const
  {
    std::string ret("CppNamespaceAlias");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nentityHash = ").append(std::to_string(entityHash))
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

typedef std::shared_ptr<CppNamespaceAlias> CppNamespaceAliasPtr;

}
}

#endif
