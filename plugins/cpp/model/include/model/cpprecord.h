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
  #pragma db on_delete(cascade)
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
struct CppRecord : CppEntity
{
  bool isAbstract = false;
  bool isPOD = false;

  std::string toString() const
  {
    std::string ret("CppRecord");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nmangledNameHash = ").append(std::to_string(mangledNameHash))
      .append("\nqualifiedName = ").append(qualifiedName)
      .append("\nisAbstract = ").append(std::to_string(isAbstract))
      .append("\nisPOD = ").append(std::to_string(isPOD));

    if (!tags.empty())
    {
      ret.append("\ntags =");
      for (const Tag& tag : tags)
        ret.append(' ' + tagToString(tag));
    }

    return ret;
  }
};

typedef std::shared_ptr<CppRecord> CppRecordPtr;

#pragma db view object(CppMemberType)
struct CppMemberTypeCount
{
  #pragma db column("count(" + CppMemberType::id + ")")
  std::size_t count;
};

#pragma db view object(CppRecord)
struct CppRecordCount
{
  #pragma db column("count(" + CppRecord::id + ")")
  std::size_t count;
};

}
}

#endif
