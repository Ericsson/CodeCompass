// $Id$
// Created by Aron Barath, 2014

#ifndef MODEL_JAVA_JAVAIMPORT_H
#define MODEL_JAVA_JAVAIMPORT_H

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/java/javaastnode.h>
#include <model/fileloc.h>

namespace cc
{
namespace model
{

#pragma db object
struct JavaImport
{
  #pragma db id auto
  int id;

  #pragma db not_null
  odb::lazy_shared_ptr<JavaAstNode> astNodePtr;

  #pragma db not_null
  odb::lazy_shared_ptr<File> file;

  long long mangledNameHash;

  std::string name;

  bool staticImport;
  bool onDemand;
  bool explicitImport;
};

typedef std::shared_ptr<JavaImport> JavaImportPtr;

} // model
} // cc

#endif // MODEL_JAVA_JAVAIMPORT_H

