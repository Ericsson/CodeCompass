#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include <cppparser/filelocutil.h>

#include "ppincludecallback.h"

namespace cc
{
namespace parser
{

PPIncludeCallback::PPIncludeCallback(
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  EntityCache& entityCache_,
  clang::Preprocessor&) :
    _ctx(ctx_),
    _cppSourceType("CPP"),
    _clangSrcMgr(astContext_.getSourceManager()),
    _fileLocUtil(astContext_.getSourceManager()),
    _entityCache(entityCache_)
{
}

PPIncludeCallback::~PPIncludeCallback()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    util::persistAll(_astNodes, _ctx.db);
    util::persistAll(_headerIncs, _ctx.db);
  });
}

model::CppAstNodePtr PPIncludeCallback::createFileAstNode(
  const model::FilePtr& file_,
  const clang::SourceRange& srcRange_)
{
  model::CppAstNodePtr astNode(new model::CppAstNode());

  astNode->astValue = file_->path;
  astNode->entityHash = util::fnvHash(std::to_string(file_->id));
  astNode->symbolType = model::CppAstNode::SymbolType::File;
  astNode->astType = model::CppAstNode::AstType::Usage;

  model::FileLoc& fileLoc = astNode->location;
  _fileLocUtil.setRange(
    srcRange_.getBegin(), srcRange_.getEnd(), fileLoc.range);
  fileLoc.file = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(srcRange_.getBegin()));

  astNode->id = model::createIdentifier(*astNode);

  return astNode;
}

void PPIncludeCallback::InclusionDirective(
  clang::SourceLocation hashLoc_,
  const clang::Token&,
  clang::StringRef fileName_,
  bool,
  clang::CharSourceRange filenameRange_,
  clang::OptionalFileEntryRef,
  clang::StringRef searchPath_,
  clang::StringRef,
  const clang::Module*,
  bool,
  clang::SrcMgr::CharacteristicKind)
{
  if (searchPath_.empty())
    return;

  clang::SourceLocation expLoc = _clangSrcMgr.getExpansionLoc(hashLoc_);
  clang::PresumedLoc presLoc = _clangSrcMgr.getPresumedLoc(expLoc);

  //--- Included file ---//

  std::string includedPath = searchPath_.str() + '/' + fileName_.str();
  model::FilePtr included = _ctx.srcMgr.getFile(includedPath);
  included->parseStatus = model::File::PSFullyParsed;
  if (included->type != model::File::DIRECTORY_TYPE &&
      included->type != _cppSourceType)
  {
    included->type = _cppSourceType;
    _ctx.srcMgr.updateFile(*included);
  }

  //--- Includer file ---//

  std::string includerPath = presLoc.getFilename();
  model::FilePtr includer = _ctx.srcMgr.getFile(includerPath);
  includer->parseStatus = model::File::PSFullyParsed;
  if (includer->type != model::File::DIRECTORY_TYPE &&
      includer->type != _cppSourceType)
  {
    includer->type = _cppSourceType;
    _ctx.srcMgr.updateFile(*includer);
  }

  //--- CppAstNode ---//

  model::CppAstNodePtr fileNode =
    createFileAstNode(included, filenameRange_.getAsRange());

  if (_entityCache.insert(*fileNode))
    _astNodes.push_back(fileNode);

  model::CppHeaderInclusionPtr inclusion(new model::CppHeaderInclusion);
  inclusion->includer = includer;
  inclusion->included = included;

  _headerIncs.push_back(inclusion);
}

} // parser
} // cc
