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
    std::string ret("CppEnumConstant");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nvalue = ").append(std::to_string(value));

    if (!tags.empty())
    {
      ret.append("\ntags =");
      for (const Tag& tag : tags)
        ret.append(' ' + tagToString(tag));
    }

    return ret;
  }
};

typedef std::shared_ptr<CppEnumConstant> CppEnumConstantPtr;

#pragma db object
struct CppEnum : CppEntity
{
  std::vector<odb::lazy_shared_ptr<CppEnumConstant>> enumConstants;

  std::string toString() const
  {
    std::string ret("CppEnum");

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

typedef std::shared_ptr<CppEnum> CppEnumPtr;

#pragma db view \
  object(CppEnum) object(CppEnumConstant = EnumConst : CppEnum::enumConstants)
struct CppEnumConstantsCount
{
  #pragma db column("count(" + EnumConst::id + ")")
  std::size_t count;
};

}
}

#endif
