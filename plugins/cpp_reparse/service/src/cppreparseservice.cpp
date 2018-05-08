#include <chrono>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <util/logutil.h>

#include <service/cppreparseservice.h>
#include <service/reparser.h>

#include "astcache.h"
#include "asthtml.h"
#include "infotree.h"

namespace
{

typedef odb::query<cc::model::CppAstNode> AstQuery;

} // namespace (anonymous)

namespace cc
{
namespace service
{

namespace language
{

using namespace cc::service::reparse;
using namespace clang;
using namespace clang::tooling;

CppReparseServiceHandler::CppReparseServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /* datadir_ */,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _config(context_.options)
{
  if (isEnabled())
  {
    size_t jobs = static_cast<size_t>(_config["jobs"].as<int>());
    size_t maxCacheSize = _config["ast-cache-limit"].as<size_t>();
    if (jobs > maxCacheSize)
    {
      LOG(warning) << "--ast-cache-limit set to smaller amount than --jobs. "
                      "To prevent frequent cache misses, the cache size was "
                      "increased to " << jobs;
      maxCacheSize = jobs;
    }

    _astCache = std::make_shared<ASTCache>(maxCacheSize);
    _reparser = std::make_unique<CppReparser>(_db, _astCache);
  }
}

CppReparseServiceHandler::~CppReparseServiceHandler() = default;

bool CppReparseServiceHandler::isEnabled()
{
  return !_config["disable-cpp-reparse"].as<bool>();
}

void CppReparseServiceHandler::getAsHTML(
  std::string& return_,
  const core::FileId& fileId_)
{
  if (!isEnabled())
  {
    return_ = "Reparse capabilities has been disabled at server start.";
    return;
  }

  auto result = _reparser->getASTForTranslationUnitFile(fileId_);
  if (std::string* err = boost::get<std::string>(&result))
  {
    return_ = "The AST could not be obtained. " + *err + " - The server log "
              "might contain more details.";
    return;
  }
  std::shared_ptr<ASTUnit> AST = boost::get<std::shared_ptr<ASTUnit>>(result);

  ASTHTMLActionFactory htmlFactory;
  htmlFactory.newASTConsumer()->HandleTranslationUnit(AST->getASTContext());
  return_ = htmlFactory.str();
}

void CppReparseServiceHandler::getAsHTMLForNode(
  std::string& return_,
  const core::AstNodeId& nodeId_)
{
  if (!isEnabled())
  {
    return_ = "Reparse capabilities has been disabled at server start.";
    return;
  }

  model::CppAstNodePtr astNode;
  _transaction([&, this](){
    astNode = _db->query_one<model::CppAstNode>(
      AstQuery::id == std::stoull(nodeId_));
    astNode->location.file.load();
  });

  if (!astNode)
  {
    LOG(warning) << "No location file found for AST node #" << nodeId_;
    return_ = "Invalid AST node given, no related file could be found.";
    return;
  }

  core::FileId fileId_ = std::to_string(astNode->location.file.object_id());
  auto result = _reparser->getASTForTranslationUnitFile(fileId_);
  if (std::string* err = boost::get<std::string>(&result))
  {
    return_ = "The AST could not be obtained. " + *err + " - The server log "
      "might contain more details.";
    return;
  }
  std::shared_ptr<ASTUnit> AST = boost::get<std::shared_ptr<ASTUnit>>(result);

  // TODO: What if clicking on something whose location is in a header?!
  /* TODO: What to do with elements that don't have an AST node in the database?
   * One possible way would be to ensure the user can also make a request on a
   * source range selection. TODO: This shall be handled later, not priority now.
   */
  LOG(debug) << "File: " << astNode->location.file->path;
  LOG(debug) << "Range: " << astNode->location.range.start.line << ":"
             << astNode->location.range.start.column << " -> "
             << astNode->location.range.end.line << ":"
             << astNode->location.range.end.column;

  ASTHTMLActionFactory htmlFactory;
  ASTContext& context = AST->getASTContext();
  htmlFactory.newASTConsumerForNode(context, astNode)
    ->HandleTranslationUnit(context);
  return_ = htmlFactory.str();
}

void CppReparseServiceHandler::getBasic(
  ASTNodeBasic& return_,
  const core::FileId& fileId_)
{
  if (!isEnabled())
  {
    LOG(warning) << "Reparse capabilities has been disabled at server start.";
    return;
  }

  auto result = _reparser->getASTForTranslationUnitFile(fileId_);
  if (std::string* err = boost::get<std::string>(&result))
  {
    LOG(warning) << "The AST could not be obtained. " + *err;
    return;
  }
  std::shared_ptr<ASTUnit> AST = boost::get<std::shared_ptr<ASTUnit>>(result);

  ASTInfoTree extractor(_astCache.get(), fileId_, AST->getASTContext());

  // The first node that is visited is the TranslationUnit's root node.
  boost::optional<ASTInfoTree::Node> res = extractor.getNodeDataForId(1);
  if (!res)
  {
    LOG(warning) << "The node couldn't have been located.";
    return;
  }

  return_.visitId = static_cast<int64_t>(res->_visitId);
  return_.type = extractor.getNodeType(*res);
  return_.hasChildren = res->_hasChildren;
}

void CppReparseServiceHandler::getBasicForNode(
  ASTNodeBasic& return_,
  const cc::service::core::AstNodeId& nodeId_)
{
  if (!isEnabled())
  {
    LOG(warning) << "Reparse capabilities has been disabled at server start.";
    return;
  }

  model::CppAstNodePtr astNode;
  _transaction([&, this](){
    astNode = _db->query_one<model::CppAstNode>(
      AstQuery::id == std::stoull(nodeId_));
    astNode->location.file.load();
  });

  if (!astNode)
  {
    LOG(warning) << "No location file found for AST node #" << nodeId_;
    return;
  }

  core::FileId fileId_ = std::to_string(astNode->location.file.object_id());
  auto result = _reparser->getASTForTranslationUnitFile(fileId_);
  if (std::string* err = boost::get<std::string>(&result))
  {
    LOG(warning) <<  "The AST could not be obtained. " + *err;
    return;
  }
  std::shared_ptr<ASTUnit> AST = boost::get<std::shared_ptr<ASTUnit>>(result);

  ASTInfoTree extractor(_astCache.get(), fileId_, AST->getASTContext());
  boost::optional<ASTInfoTree::Node> res =
    extractor.getNodeDataForAstNode(astNode);
  if (!res)
  {
    LOG(warning) << "The node couldn't have been located.";
    return;
  }

  return_.visitId = static_cast<int64_t>(res->_visitId);
  return_.type = extractor.getNodeType(*res);
  return_.hasChildren = res->_hasChildren;
}

void CppReparseServiceHandler::getDetail(
  ASTNodeDetail& return_,
  const core::FileId& fileId_,
  const int64_t visitId_)
{
  if (!isEnabled())
  {
    LOG(warning) << "Reparse capabilities has been disabled at server start.";
    return;
  }

  auto result = _reparser->getASTForTranslationUnitFile(fileId_);
  if (std::string* err = boost::get<std::string>(&result))
  {
    LOG(warning) << "The AST could not be obtained. " + *err;
    return;
  }
  std::shared_ptr<ASTUnit> AST = boost::get<std::shared_ptr<ASTUnit>>(result);

  ASTInfoTree extractor(_astCache.get(), fileId_, AST->getASTContext());

  auto node = extractor.getNodeDataForId(static_cast<size_t>(visitId_));
  if (!node)
  {
    LOG(warning) << "Node " << visitId_ << " not found.";
    return;
  }

  // Traverse the children nodes of the input node to fetch what (if any) types
  // of children it has.
  auto children = extractor.getChildrenForNode(*node);
  for (auto child : children)
  {
    ASTNodeBasic basicRecord;
    basicRecord.visitId = static_cast<int64_t>(child._visitId);
    basicRecord.type = extractor.getNodeType(child);
    basicRecord.hasChildren = child._hasChildren;

    return_.children.push_back(basicRecord);
  }

  // TODO: Populate the details better based on the type of the node.
  // (This is a basic dummy implementation for now.)
  return_.otherStuff = extractor.dummyGetDetails(*node);
}

} // namespace language
} // namespace service
} // namespace cc
