// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVATYPE_H
#define MODEL_JAVA_JAVATYPE_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>
#include <model/java/javamember.h>
#include <model/java/javaenumconstant.h>
#include <model/java/javaannotation.h>

namespace cc
{
namespace model
{

struct JavaFunction;

#pragma db object
struct JavaType
{
  typedef long long pktype;

  // #pragma db id auto
  // pktype id;

  #pragma db null
  JavaAstNodePtr astNodePtr; // TODO: rename to astNode

  #pragma db id
  long long mangledNameHash;

  std::string name;
  std::string qualifiedName;

  #pragma db null
  odb::lazy_shared_ptr<JavaType> superClass;

  #pragma db inverse(type) null
  std::vector<odb::lazy_shared_ptr<JavaMember>> fields;

  #pragma db inverse(type) null
  std::vector<odb::lazy_weak_ptr<JavaFunction>> functions;

  bool isEnum;
  bool isGeneric;

  #pragma db inverse(enumType) null
  std::vector<odb::lazy_shared_ptr<JavaEnumConstant>> enumConstants;

  #pragma db null
  odb::lazy_shared_ptr<JavaType> genericImpl;
};

// TODO: HACK
typedef JavaType JavaEnum;

typedef std::shared_ptr<JavaType> JavaTypePtr;

} // model
} // cc

#include <model/java/javafunction.h>

#endif // MODEL_JAVA_JAVATYPE_H

