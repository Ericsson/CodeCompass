/*
 * symbolhandler.h
 *
 *  Created on: Sep 4, 2013
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_SYMBOLHANDLER_H
#define CODECOMPASS_SYMBOLHANDLER_H

#include <memory>

#include "odb/database.hxx"

#include "language-api/language_types.h"

#include "model/cxx/cppastnode.h"
#include "model/cxx/cppvariable.h"
#include "model/cxx/cppfunction.h"

#include "odbquery.h"

namespace cc
{ 
namespace service
{  
namespace language
{

class /*Cpp*/SymbolHandler // TODO: public SymbolHandler
{
public:
  static std::unique_ptr<SymbolHandler> getHandler(
    std::shared_ptr<odb::database> db, model::CppAstNode::SymbolType symbolType);

  SymbolHandler(std::shared_ptr<odb::database> db)
    : db(db), query(db)
  {
  }

  virtual std::vector<InfoNode> getInfoTree(
    const model::CppAstNode& astNode) = 0;

  virtual std::vector<InfoNode> getSubInfoTree(
    const model::CppAstNode& astNode, const InfoQuery& query) = 0;

  virtual std::string getInfoBoxText(const model::CppAstNode& astNode);

  virtual std::vector<model::CppAstNode> getCallers(const model::HashType mangledNameHash,
    const model::FileId& fileId);
    
  virtual std::vector<InfoNode> getPointerAnalysisInfoTree(const model::CppAstNode& astNode);
  
  virtual std::string getPointerAnalysisDiagram(const model::CppAstNode& astNode);
    
  virtual ~SymbolHandler()
  {
  }

protected:

  std::shared_ptr<odb::database> db;
  CppOdbQuery query;
};

class CppFileHandler // TODO : public FileHandler
{
public:
  CppFileHandler(std::shared_ptr<odb::database> db)
    : db(db), query(db)
  {
  }

  std::vector<InfoNode> getInfoTreeForFile(
    const model::FileId);

  std::vector<InfoNode> getSubInfoTreeForFile(
    const model::FileId&,
    const InfoQuery&);

private:
  enum class SubQuery
  {
    includes = 1,
    macros,
    types,
    functions,
    globals
  };

  std::shared_ptr<odb::database> db;
  CppOdbQuery query;
};

InfoNode makeInfoNode(
  std::shared_ptr<odb::database> db,
  std::vector<std::string> category,
  const model::CppVariable& variable);

InfoNode makeInfoNode(
  std::vector<std::string> category,
  std::string label_,
  std::string value_,
  AstNodeInfo astValue_ = AstNodeInfo());

InfoNode makeInfoNode(
  std::vector<std::string> category,
  const model::CppAstNode& call,
  const model::CppFunction& function);

InfoNode makeInfoNodeOfFunction(
  const model::CppFunction& function);

InfoNode makeInfoQueryNode(
  std::vector<std::string> category,
  int queryId,
  std::vector<std::string> filters = std::vector<std::string>());

std::vector<InfoNode> makeHitCountQueryNodes(
  std::shared_ptr<odb::database> db,
  const std::vector<model::AstCountGroupByFiles>& hitcounts,
  int queryId,
  std::vector<std::string> filters = std::vector<std::string>());

std::vector<InfoNode> makeHitCountQueryNodes(
  std::shared_ptr<odb::database> db,
  odb::result<model::AstCountGroupByFiles> hitcounts,
  int queryId,
  std::vector<std::string> filters = std::vector<std::string>());

} // language
} // service
} // cc

#endif // CODECOMPASS_SYMBOLHANDLER_H

