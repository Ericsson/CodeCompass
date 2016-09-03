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

  bool isStatic = false;
};

typedef std::shared_ptr<CppMemberType> CppMemberTypePtr;

#pragma db object
struct CppType : CppEntity
{
  bool isAbstract = false;
  bool isPOD = false;
};

typedef std::shared_ptr<CppType> CppTypePtr;

}
}

#endif
