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
  enum Context {
    TOP_LEVEL,
    NAMESPACE,
    RECORD,
    FUNCTION,
    OTHER
  };

  bool isAbstract = false;
  bool isPOD = false;
  bool isLambda = false;

  // Context defines where the CppRecord is located in (e.g. in a namespace).
  // Context = RECORD means it is nested within another record.
  Context context = Context::OTHER;

  std::string toString() const
  {
    std::string ret("CppRecord");

    ret
      .append("\nid = ").append(std::to_string(id))
      .append("\nentityHash = ").append(std::to_string(entityHash))
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

  std::string getContextString() const
  {
    switch (context)
    {
      case Context::TOP_LEVEL: return "Top Level";
      case Context::NAMESPACE: return "Namespace";
      case Context::RECORD: return "Record";
      case Context::FUNCTION: return "Function";
      case Context::OTHER: return "Other";
    }
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

#pragma db view \
  object(CppMemberType) \
  object(CppAstNode : CppMemberType::memberAstNode == CppAstNode::id)
struct CppMemberTypeAstView
{
  #pragma db column(CppMemberType::typeHash)
  std::uint64_t typeHash;
};

}
}

#endif
