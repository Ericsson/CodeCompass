// $Id$
// Created by Aron Barath, 2014

#ifndef MODEL_JAVA_JAVAANNOTATION_H
#define MODEL_JAVA_JAVAANNOTATION_H

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

#pragma db object
struct JavaAnnotation
{
  #pragma db id auto
  int id;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaAstNode> astNodePtr;

  long long mangledNameHash;

  std::string name;

  // TODO
  //odb::lazy_shared_ptr<JavaType> annotationType;

  // TODO
  //std::vector<odb::lazy_shared_ptr<JavaAnnotationParam>> params;
};

typedef std::shared_ptr<JavaAnnotation> JavaAnnotationPtr;

} // model
} // cc

#endif // MODEL_JAVA_JAVAANNOTATION_H

