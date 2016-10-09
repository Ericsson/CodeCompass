#ifndef CC_MODEL_CPPENUM_H
#define CC_MODEL_CPPENUM_H

#include "cppentity.h"

namespace cc
{
namespace model
{

#pragma db object
struct CppEnumConstant : CppEntity
{
  int value;

  std::string toString() const
  {
    return std::string("CppEnumConstant")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nvalue = ").append(std::to_string(value));
  }
};

typedef std::shared_ptr<CppEnumConstant> CppEnumConstantPtr;

#pragma db object
struct CppEnum : CppEntity
{
  std::vector<odb::lazy_shared_ptr<CppEnumConstant>> enumConstants;

  std::string toString() const
  {
    return std::string("CppEnum")
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName);
  }
};

typedef std::shared_ptr<CppEnum> CppEnumPtr;

}
}

#endif
