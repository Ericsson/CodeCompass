/*
 * cppenum.h
 *
 *  Created on: Sep 20, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CPPENTITY_H_
#define MODEL_CPPENTITY_H_

#include <exception>

#include "cppastnode.h"
#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

namespace cc
{
namespace model
{

struct CppEnum;
struct CppEnumConstant;
struct CppFunction;
struct CppFunctionPointer;
struct CppNamespace;
struct CppMacro;
struct CppType;
struct CppTypedef;
struct CppVariable;
struct CppImplicit;

struct CppEntityVisitor
{
  virtual ~CppEntityVisitor(){}

  virtual void visit(CppEnum&)            = 0;
  virtual void visit(CppEnumConstant&)    = 0;
  virtual void visit(CppFunction&)        = 0;
  virtual void visit(CppFunctionPointer&) = 0;
  virtual void visit(CppNamespace&)       = 0;
  virtual void visit(CppMacro&)           = 0;
  virtual void visit(CppType&)            = 0;
  virtual void visit(CppTypedef&)         = 0;
  virtual void visit(CppVariable&)        = 0;
  virtual void visit(CppImplicit&)        = 0;
};

/**
 * This class represents a typedef in c++
 */
#pragma db object polymorphic
struct CppEntity
{
  typedef int pktype;

  virtual ~CppEntity() {}

  virtual void accept(CppEntityVisitor&) = 0;

  #pragma db id auto
  pktype id;

  #pragma db null
  odb::nullable<CppAstNode::pktype> astNodeId;

  HashType mangledNameHash = 0;

  std::string name;
  std::string qualifiedName;

#ifndef NO_INDICES
  #pragma db index member(astNodeId)
  #pragma db index member(mangledNameHash)
#endif
};

/**
 * An entity which has a type like variables or
 * functions (return types in this case)
 */
#pragma db object
struct CppTypedEntity : CppEntity
{
  void accept(CppEntityVisitor&)
  {
    throw std::runtime_error("CppTypedEntity::accept called");
  }

  unsigned long long typeHash;
  std::string qualifiedType;
};


typedef std::shared_ptr<CppEntity>      CppEntityPtr;
typedef std::shared_ptr<CppTypedEntity> CppTypedEntityPtr;

#pragma db view \
  object(CppEntity) \
  object(CppAstNode : CppAstNode::id == CppEntity::astNodeId)
struct CppQualifiedName
{
  #pragma db column("CppEntity.typeid") // type("TEXT")
  std::string typeId;
  
  #pragma db column(CppEntity::qualifiedName)
  std::string qualifiedName;
  
  #pragma db column(CppAstNode::location.file)
  odb::nullable<FileId> file;
  
  #pragma db column(CppAstNode::location.range.start.line)
  Position::postype start_line;
  
  #pragma db column(CppAstNode::location.range.start.column)
  Position::postype start_col;
  
  #pragma db column(CppAstNode::location.range.end.line)
  Position::postype end_line;
  
  #pragma db column(CppAstNode::location.range.end.column)
  Position::postype end_col;
};

#pragma db view object(CppEntity)
struct CppEntityAstNodeId
{
  #pragma db column(CppEntity::id)
  CppEntity::pktype id;

  #pragma db column(CppEntity::astNodeId)
  odb::nullable<CppAstNode::pktype> astNodeId;
};

#pragma db view \
  object(CppEntity) \
  object(CppAstNode : CppAstNode::id == CppEntity::astNodeId)
struct CppEntityByHash
{
  #pragma db column(CppEntity::id)
  CppEntity::pktype id;

  #pragma db column(CppEntity::mangledNameHash)
  HashType mangledNameHash;

  #pragma db column(CppEntity::astNodeId)
  CppAstNode::pktype astNodeId;

  #pragma db column(CppAstNode::astType)
  CppAstNode::AstType astType;
};

} // model
} // cc

#endif /* MODEL_CPPENTITY_H_ */
