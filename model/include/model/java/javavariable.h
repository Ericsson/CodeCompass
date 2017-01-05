// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAVARIABLE_H
#define MODEL_JAVA_JAVAVARIABLE_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>
#include <model/java/javatype.h>

namespace cc
{
namespace model
{

struct JavaFunction;

#pragma db object
struct JavaVariable
{
  #pragma db id auto
  int id;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaType> type;

  #pragma db not_null
  JavaAstNodePtr astNodePtr; // TODO: rename to astNode

  unsigned long long mangledNameHash;

  std::string name;
  std::string qualifiedName;

  #pragma db null
  odb::lazy_shared_ptr<JavaFunction> paramInFunc;

  #pragma db null
  odb::lazy_shared_ptr<JavaFunction> localInFunc;
};

typedef std::shared_ptr<JavaVariable> JavaVariablePtr;

} // model
} // cc

#include <model/java/javafunction.h>

#endif // MODEL_JAVA_JAVAVARIABLE_H

