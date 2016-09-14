// $Id$
// Created by Aron Barath, 2013

#ifndef MODEL_JAVA_JAVAMEMBER_H
#define MODEL_JAVA_JAVAMEMBER_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>
#include <model/java/javamodifiers.h>

namespace cc
{
namespace model
{

struct JavaType;

#pragma db object
struct JavaMember
{
  #pragma db id auto
  int id;

  std::string name;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaType> type;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaType> fieldType;

  #pragma db not_null
  JavaAstNodePtr astNodePtr; // TODO: rename to astNode

  int modifiers;
};

typedef std::shared_ptr<JavaMember> JavaMemberPtr;

} // model
} // cc

#include <model/java/javatype.h>

#endif // MODEL_JAVA_JAVAMEMBER_H

