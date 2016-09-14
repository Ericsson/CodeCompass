// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAENUMCONSTANT_H
#define MODEL_JAVA_JAVAENUMCONSTANT_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>

namespace cc
{
namespace model
{

struct JavaType;

#pragma db object
struct JavaEnumConstant
{
  #pragma db id auto
  int id;

  #pragma db not_null
  JavaAstNodePtr astNodePtr;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaType> enumType;

  long long mangledNameHash;

  std::string name;
  std::string qualifiedName;
};

typedef std::shared_ptr<JavaEnumConstant> JavaEnumConstantPtr;

} // model
} // cc

#include <model/java/javatype.h>

#endif // MODEL_JAVA_JAVAENUMVALUECONSTANT_H

