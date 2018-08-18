#include <chrono>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cpptype.h>
#include <model/cpptype-odb.hxx>
#include <model/file-odb.hxx>
#include <model/fileloc.h>
#include <model/position.h>

#include <util/logutil.h>

#include <service/cppservice.h>
#include <service/cppreparseservice.h>
#include <service/reparser.h>

#include "astcache.h"
#include "asthtml.h"
#include "typespecialmembers.h"

namespace
{

typedef odb::query<cc::model::CppAstNode> AstQuery;
typedef odb::query<cc::model::CppMemberType> MemTypeQuery;
typedef odb::query<cc::model::File> FileQuery;

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
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _datadir(datadir_),
    _context(context_)
{
  if (isEnabled())
  {
    size_t jobs = static_cast<size_t>(_context.options["jobs"].as<int>());
    size_t maxCacheSize = _context.options["ast-cache-limit"].as<size_t>();
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
  return !_context.options.count("disable-cpp-reparse");
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

void CppReparseServiceHandler::getSpecialMembersSource(
  std::vector<language::SourceTextFragment>& return_,
  const core::AstNodeId& astNode_)
{
  if (!isEnabled())
  {
    LOG(warning) << "Reparse capabilities has been disabled at server start.";
    return;
  }

  // Fetch the AST node that that is either the record definition specified by
  // the user, or the type definition for the class of the member function
  // the user clicked on.
  model::CppAstNodePtr astNode;
  _transaction([&, this]() {
    astNode = _db->query_one<model::CppAstNode>(
      AstQuery::id == std::stoull(astNode_));
    if (!astNode)
      return;

    if (astNode->symbolType == model::CppAstNode::SymbolType::Function)
    {
      auto memberType = _db->query_one<model::CppMemberType>(
        MemTypeQuery::memberAstNode == astNode->id &&
        MemTypeQuery::kind == model::CppMemberType::Kind::Method);
      astNode = nullptr;
      if (!memberType)
      {
        LOG(warning) << "Couldn't fetch this method as a member ... ?!";
        return;
      }

      astNode = _db->query_one<model::CppAstNode>(
        AstQuery::mangledNameHash == memberType->typeHash &&
        AstQuery::symbolType == model::CppAstNode::SymbolType::Type &&
        AstQuery::astType == model::CppAstNode::AstType::Definition);

      if (!astNode)
      {
        LOG(warning)
          << "Couldn't find the type definition for the member function";
        return;
      }
    }
  });

  if (!astNode)
  {
    LOG(warning) << "No location file found for AST node #" << astNode_;
    return;
  }

  TypeSpecialMemberPrinter::DefinitionSearchFunction defSearch =
    [&](const std::string& filePath_, const model::Range& range_)
      -> model::FileLoc
    {
      model::FileLoc res;
      _transaction([&] {
        core::FilePosition thriftPosition;

        core::Position pos;
        pos.line = range_.start.line;
        pos.column = range_.start.column;
        thriftPosition.__set_pos(pos);

        model::FilePtr file = _db->query_one<model::File>(
          FileQuery::path == filePath_);
        thriftPosition.__set_file(std::to_string(file->id));

        // Emulate the user's click of "Jump to def" on the forward declaration.
        CppServiceHandler cppService(_db, _datadir, _context);
        AstNodeInfo declAstNodeInfo;
        cppService.getAstNodeInfoByPosition(declAstNodeInfo, thriftPosition);

        std::map<std::string, std::int32_t> applicableReferences;
        cppService.getReferenceTypes(applicableReferences, declAstNodeInfo.id);

        std::vector<AstNodeInfo> definitions;
        cppService.getReferences(definitions, declAstNodeInfo.id,
          applicableReferences["Definition"], {});

        if (definitions.empty())
          return;

        const auto& definitionRange = definitions.front().range;

        res.file = _db->load<model::File>(std::stoull(definitionRange.file));
        res.range.start.line = definitionRange.range.startpos.line;
        res.range.start.column = definitionRange.range.startpos.column;
        res.range.end.line = definitionRange.range.endpos.line;
        res.range.end.column = definitionRange.range.endpos.column;
      });
      return res;
    };

  TypeSpecialMemberPrinter tsmp(*_db, *_reparser, std::move(defSearch));
  return_ = tsmp.resolveMembersFor(astNode);
}

} // namespace language
} // namespace service
} // namespace cc
