// $Id$
// Created by Aron Barath, 2013

#ifndef CODECOMPASS_JAVASERVICEHELPER_UTILS_H
#define CODECOMPASS_JAVASERVICEHELPER_UTILS_H

#include <vector>
#include <limits>

#include <odb/lazy-ptr.hxx>

#include "language-api/language_types.h"

#include "javaservicehelper/odbquery.h"
#include "model/java/javaastnode.h"
#include "model/java/javatype.h"
#include "model/java/javafunction.h"
#include "langservicelib/utils.h"

namespace cc
{
namespace service
{
namespace language 
{

std::string getFileloc(const model::JavaAstNode& astNode);

AstNodeInfo createAstNodeInfo(
  const model::JavaAstNode& astNode);

AstNodeInfo createAstNodeInfo(
  const model::JavaAstNode& astNode,
  const model::JavaAstNode& defAstNode);

std::vector<model::JavaType> getInheritsFromTypes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  JavaHashType mangledNameHash);

std::vector<model::JavaType> getInheritsFromTypes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  const model::JavaAstNode& astNode);

std::vector<model::JavaType> getInheritsByTypes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  JavaHashType mangledNameHash,
  bool includeGenInst=false);

std::vector<model::JavaType> getInheritsByTypes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  const model::JavaAstNode& astNode,
  bool includeGenInst=false);

std::vector<JavaHashType> getOverrideHashes(
  std::shared_ptr<odb::database> db,
  JavaOdbQuery & query,
  const model::JavaFunction & function);

} // language
} // service
} // cc

#endif // CODECOMPASS_JAVASERVICEHELPER_UTILS_H
