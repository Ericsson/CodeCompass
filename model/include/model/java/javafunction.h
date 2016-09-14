// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAFUNCTION_H
#define MODEL_JAVA_JAVAFUNCTION_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>
#include <model/java/javamodifiers.h>
#include <model/java/javamember.h>

namespace cc
{
namespace model
{

struct JavaVariable;
struct JavaFunction;

#pragma db object
struct JavaFunction
{
  typedef int pktype;

  // #pragma db id auto
  // pktype id;

  #pragma db null
  odb::lazy_shared_ptr<JavaType> type;

  #pragma db null
  JavaAstNodePtr astNodePtr; // TODO: rename to astNode

  int modifiers;

  std::string name;

  #pragma db id
  unsigned long long mangledNameHash;

  std::string mangledName;

  #pragma db null
  std::string qualifiedName;
  std::string signature; // <function name>(<arguments>)

  bool isGeneric;
  #pragma db null
  odb::lazy_shared_ptr<JavaFunction> genericImpl;

  odb::lazy_shared_ptr<JavaType> returnType;

  #pragma db inverse(paramInFunc)
  std::vector<odb::lazy_shared_ptr<JavaVariable>> parameters;

  #pragma db inverse(localInFunc)
  std::vector<odb::lazy_shared_ptr<JavaVariable>> locals;

  // TODO
  //std::vector<odb::lazy_shared_ptr<JavaAnnotation>> annotations;
};

typedef std::shared_ptr<JavaFunction> JavaFunctionPtr;

} // model
} // cc

#include <model/java/javavariable.h>

#endif // MODEL_JAVA_JAVAFUNCTION_H

