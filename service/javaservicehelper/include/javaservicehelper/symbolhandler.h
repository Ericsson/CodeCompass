// $Id$
// Created by Aron Barath, 2013

#ifndef CODECOMPASS_SYMBOLHANDLER_H_
#define CODECOMPASS_SYMBOLHANDLER_H_

#include <memory>

#include "odb/database.hxx"

#include "language-api/language_types.h"

#include "model/java/javaastnode.h"
#include "model/java/javavariable.h"
#include "model/java/javafunction.h"

#include "odbquery.h"

namespace cc
{
namespace service
{
namespace language 
{

class SymbolHandler
{
public:
  static std::unique_ptr<SymbolHandler> getHandler(
    std::shared_ptr<odb::database> db, model::SymbolType symbolType);

  SymbolHandler(std::shared_ptr<odb::database> db)
    : db(db), query(db)
  {
  }

  virtual std::vector<InfoNode> getInfoTree(
    const model::JavaAstNode& astNode) = 0;

  virtual std::vector<InfoNode> getSubInfoTree(
    const model::JavaAstNode& astNode, const InfoQuery& query) = 0;

  virtual std::string getInfoBoxText(const model::JavaAstNode& astNode);

  virtual ~SymbolHandler()
  {
  }

  friend class JavaFileHandler;

protected:
  static model::JavaAstNodePtr loadAstNodePtr(
    const model::JavaType & javaType,
    bool genericlevel=true);

  static model::JavaAstNodePtr loadAstNodePtr(
    const model::JavaFunction & javaFunction,
    bool genericlevel=true);

  static InfoNode makeInfoNode(
    const std::vector<std::string> & category_,
    const std::string & label_,
    const std::string & value_,
    const AstNodeInfo & astValue_ = AstNodeInfo());

  static InfoNode makeInfoNodeEx(
    std::vector<std::string> category_,
    std::string label_,
    model::JavaAstNodePtr node);

  static InfoNode makeInfoNodeFromType(
    const std::vector<std::string> & category_,
    const std::string & label_,
    const model::JavaType & type_,
    model::JavaAstNodePtr node);

  static InfoNode makeInfoNodeFromCall(
    const std::vector<std::string> & category_,
    const model::JavaFunction & func_,
    const model::JavaAstNode & call_);

  static InfoNode makeInfoQueryNode(
    std::vector<std::string> category,
    int queryId,
    std::vector<std::string> filters = std::vector<std::string>());

  static std::vector<InfoNode> makeHitCountQueryNodes(
    std::shared_ptr<odb::database> db,
    odb::result<model::AstCountGroupByFilesJava> hitcounts,
    int queryId,
    std::vector<std::string> filters = std::vector<std::string>());

  std::vector<InfoNode> getUsageOf(
    unsigned long long mangledNameHash_,
    model::AstType mode_,
    int subquery_,
    std::vector<std::string> filters_);

  std::vector<InfoNode> getUsageOfInFile(
    unsigned long long mangledNameHash,
    model::AstType mode,
    model::FileId fileId);

  std::vector<InfoNode> getUsageOfInFile(
    unsigned long long mangledNameHash,
    JavaOdbQuery::AstQuery query_,
    model::FileId fileId);

  std::vector<InfoNode> getUsageOfInFile(JavaOdbQuery::AstQuery query_);

  std::vector<InfoNode> getAnnotationsFor(const model::JavaAstNode & astNode);

  static std::string getFunctionSignature(
    const model::JavaFunction & func_,
    bool withRetType_);

  std::shared_ptr<odb::database> db;
  JavaOdbQuery query;
};

class JavaFileHandler // TODO : public FileHandler
{
public:
  JavaFileHandler(std::shared_ptr<odb::database> db)
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
    imports = 1,
    types,
  };

  std::vector<InfoNode> getImports(model::FileId fid_);

  std::vector<InfoNode> getTypes(model::FileId fid_);

  void splitToCategories(const std::string & str_,
    std::size_t start_,
    std::vector<std::string> & cat_);

  std::size_t findNext(const std::string & str_,
    char ch_, std::size_t pos_);

  std::string getPackageNameFromTypeName(const std::string & typename_);

  std::vector<InfoNode> queryAstNodes(
    JavaOdbQuery::AstQuery astQuery_,
    const std::vector<std::string>& category_,
    std::size_t prefixlen_,
    bool split_);

  std::vector<InfoNode> queryMembersOfTypes(
    odb::result<model::JavaAstNode> types_,
    const std::vector<std::string>& category_,
    std::size_t prefixlen_,
    bool split_);

  std::shared_ptr<odb::database> db;
  JavaOdbQuery query;
};

} // language
} // service
} // cc

#endif /* SYMBOLHANDLER_H_ */
