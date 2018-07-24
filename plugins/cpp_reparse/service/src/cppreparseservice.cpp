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

} // namespace language
} // namespace service
} // namespace cc
