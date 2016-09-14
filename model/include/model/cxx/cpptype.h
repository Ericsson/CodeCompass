#ifndef MODEL_CXX_TYPE_H
#define MODEL_CXX_TYPE_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/fileloc.h>
#include <model/cxx/cppentity.h>

namespace cc
{
namespace model
{

enum Visibility
{
  Private, Protected, Public
};

#pragma db object
struct CppMemberType
{
  typedef int pktype;

  enum Kind
  {
    Field, Method
  };

  #pragma db id auto
  pktype id;

  /** The container type, which own the member. */
  HashType typeHash;

  /** The owned member. */
  odb::lazy_shared_ptr<CppAstNode> memberAstNode;
  HashType memberTypeHash;
  
  Kind kind;
  Visibility visibility;

  bool isStatic = false;
};

/**
 * An instance of this class should be an AST node which represents
 * the definition of a user-defined types or a primitive built-int type
 */
#pragma db object
struct CppType : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  bool isAbstract = false;
  bool isPOD      = false;
};

typedef std::shared_ptr<CppMemberType> CppMemberTypePtr;
typedef std::shared_ptr<CppType> CppTypePtr;

} // model
} // cc

#endif//MODEL_CXX_TYPE_H
