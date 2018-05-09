#ifndef CC_MODEL_CPPTYPE_H
#define CC_MODEL_CPPTYPE_H

#include "common.h"
#include "cppentity.h"

namespace cc
{
namespace model
{

typedef int CppMemberTypeId;

#pragma db object
struct CppMemberType
{
  enum Kind
  {
    Field,
    Method
  };

  #pragma db id auto
  CppMemberTypeId id;

  std::uint64_t typeHash;

  #pragma db unique
  odb::lazy_shared_ptr<CppAstNode> memberAstNode;
  std::uint64_t memberTypeHash;

  Kind kind;
  Visibility visibility;

  std::string toString() const
  {
    return std::string("CppMemberType")
      .append("\nid = ").append(std::to_string(id))
      .append("\ntypeHash = ").append(std::to_string(typeHash))
      .append("\nmemberTypeHash = ").append(std::to_string(memberTypeHash))
      .append("\nkind = ").append(kind == Field ? "Field" : "Method")
      .append("\nvisibility = ").append(visibilityToString(visibility));
  }
};

typedef std::shared_ptr<CppMemberType> CppMemberTypePtr;

#pragma db object
struct CppType : CppEntity
{
  bool isAbstract = false;
  bool isPOD = false;

  // Size and alignment is expressed in C++ standard bytes (sizeof(char)).
  // Default value 0 means it couldn't have been obtained from Clang.
  std::size_t size = 0;
  std::size_t alignment = 0;

  std::string toString() const
  {
    std::string ret("CppType");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nisAbstract = ").append(std::to_string(isAbstract))
      .append("\nisPOD = ").append(std::to_string(isPOD));

    ret.append("\nsize = ");
    if (size == 0)
      ret.append("unknown");
    else
      ret.append(std::to_string(size)).append(" bytes");

    ret.append("\nalignment = ");
    if (alignment > 1)
      ret.append(std::to_string(alignment)).append(" bytes");
    else
      ret.append("any");

    if (!tags.empty())
    {
      ret.append("\ntags =");
      for (const Tag& tag : tags)
        ret.append(' ' + tagToString(tag));
    }

    return ret;
  }
};

typedef std::shared_ptr<CppType> CppTypePtr;

#pragma db view object(CppMemberType)
struct CppMemberTypeCount
{
  #pragma db column("count(" + CppMemberType::id + ")")
  std::size_t count;
};

#pragma db view object(CppType)
struct CppTypeCount
{
  #pragma db column("count(" + CppType::id + ")")
  std::size_t count;
};

}
}

#endif
