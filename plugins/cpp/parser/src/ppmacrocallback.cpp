#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include "filelocutil.h"
#include "ppmacrocallback.h"

namespace
{

clang::PresumedLoc getPresumedLocation(
  const clang::SourceManager &sm,
  clang::SourceLocation loc)
{
  auto expLoc = sm.getExpansionLoc(loc);
  return sm.getPresumedLoc(expLoc);
}

}

namespace cc
{
namespace parser
{

PPMacroCallback::PPMacroCallback(
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  std::unordered_map<model::CppAstNodeId, std::uint64_t>& mangledNameCache_,
  clang::Preprocessor& pp_):
  _ctx(ctx_),
  _pp(pp_),
  _cppSourceType("CPP"),
  _clangSrcMgr(astContext_.getSourceManager()),
  _fileLocUtil(astContext_.getSourceManager()),
  _mangledNameCache(mangledNameCache_)
{
}

PPMacroCallback::~PPMacroCallback()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    persistAll(_astNodes);
    persistAll(_macros);
    persistAll(_macrosExpansion);
  });
}

void PPMacroCallback::MacroExpands(
  const clang::Token& macroNameTok_,
  const clang::MacroDefinition& md_,
  clang::SourceRange range_,
  const clang::MacroArgs*)
{
  if (disabled)
    return;

  clang::SourceLocation loc = macroNameTok_.getLocation();
  if (!loc.isValid() || !loc.isFileID())
      return;

  const clang::MacroInfo* mi = md_.getMacroInfo();

  if (!mi || getMangledName(mi).empty())
    return;

  const char *begin = _pp.getSourceManager().getCharacterData(range_.getBegin());
  int len = _pp.getSourceManager().getCharacterData(range_.getEnd()) - begin +
    clang::Lexer::MeasureTokenLength(range_.getEnd(),
      _pp.getSourceManager(), _pp.getLangOpts());

  std::string copy(begin, len);
  begin = copy.c_str();
  clang::Lexer lex(loc, _pp.getLangOpts(), begin, begin, begin + len);
  std::vector<clang::Token> tokens;

  // Lex all the tokens in raw mode, to avoid entering #includes or expanding
  // macros.
  clang::Token tok;
  do
  {
    lex.LexFromRawLexer(tok);

    // If this is a # at the start of a line, discard it from the token stream.
    // We don't want the re-preprocess step to see #defines, #includes or other
    // preprocessor directives.
    if (tok.is(clang::tok::hash) && tok.isAtStartOfLine())
      continue;

    // If this is a ## token, change its kind to unknown so that repreprocessing
    // it will not produce an error.
    if (tok.is(clang::tok::hashhash))
      tok.setKind(clang::tok::unknown);

    // If this raw token is an identifier, the raw lexer won't have looked up
    // the corresponding identifier info for it. Do this now so that it will be
    // macro expanded when we re-preprocess it.
    if (tok.is(clang::tok::raw_identifier))
      _pp.LookUpIdentifierInfo(tok);

    tokens.push_back(tok);
  } while (!tok.is(clang::tok::eof));

  // Temporarily change the diagnostics object so that we ignore any generated
  // diagnostics from this pass.
  clang::DiagnosticsEngine TmpDiags(
    _pp.getDiagnostics().getDiagnosticIDs(),
    &_pp.getDiagnostics().getDiagnosticOptions(),
    new clang::IgnoringDiagConsumer);
  TmpDiags.setSourceManager(&_pp.getSourceManager());

  disabled = true;
  clang::DiagnosticsEngine *OldDiags = &_pp.getDiagnostics();
  _pp.setDiagnostics(TmpDiags);
  _pp.EnterTokenStream(tokens.data(), tokens.size(), false, false);

  std::string expansion;
  _pp.Lex(tok);
  while (tok.isNot(clang::tok::eof))
  {
    // If the tokens were already space separated, or if they must be to avoid
    // them being implicitly pasted, add a space between them.
    if (tok.hasLeadingSpace())
        expansion += ' ';

    // Escape any special characters in the token text.
    expansion += _pp.getSpelling(tok);
    _pp.Lex(tok);
  }

  _pp.setDiagnostics(*OldDiags);
  disabled = false;

  model::CppAstNodePtr astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::Usage;
  addFileLoc(astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());
  astNode->id = model::createIdentifier(*astNode);

  if (insertToCache(astNode))
  {
    _astNodes.push_back(astNode);

    model::CppMacroExpansionPtr mExp(new model::CppMacroExpansion);
    mExp->astNodeId = astNode->id;
    mExp->expansion = expansion;

    _macrosExpansion.push_back(mExp);
  }
}

void PPMacroCallback::MacroDefined(
  const clang::Token& macroNameTok_,
  const clang::MacroDirective* md_)
{
  const clang::MacroInfo* mi = md_->getMacroInfo();

  if (!mi || isBuiltInMacro(mi) || getMangledName(mi).empty())
    return;

  model::CppAstNodePtr astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::Definition;
  addFileLoc(astNode, mi->getDefinitionLoc(), mi->getDefinitionEndLoc());
  astNode->id = model::createIdentifier(*astNode);

  if (insertToCache(astNode))
  {
    _astNodes.push_back(astNode);

    model::CppMacroPtr macro = std::make_shared<model::CppMacro>();
    macro->astNodeId = astNode->id;
    macro->mangledNameHash = astNode->mangledNameHash;
    macro->name = astNode->astValue;
    macro->qualifiedName = astNode->astValue;
    _macros.push_back(macro);
  }
}

void PPMacroCallback::MacroUndefined(
  const clang::Token& macroNameTok_,
  const clang::MacroDefinition& md_)
{
  const clang::MacroInfo* mi = md_.getMacroInfo();

  if (!mi || isBuiltInMacro(mi) || getMangledName(mi).empty())
    return;

  auto astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::UnDefinition;
  addFileLoc(astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());
  astNode->id = model::createIdentifier(*astNode);

  if (insertToCache(astNode))
    _astNodes.push_back(astNode);
}

model::CppAstNodePtr PPMacroCallback::createMacroAstNode(
  const clang::Token& macroNameTok_,
  const clang::MacroInfo* mi_)
{
  model::CppAstNodePtr astNode(new model::CppAstNode);

  astNode->astValue = macroNameTok_.getIdentifierInfo()->getName().str();
  astNode->mangledName = getMangledName(mi_);
  astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
  astNode->symbolType = model::CppAstNode::SymbolType::Macro;

  return astNode;
}

void PPMacroCallback::addFileLoc(
  model::CppAstNodePtr& astNode,
  const clang::SourceLocation& start,
  const clang::SourceLocation& end)
{
  if (start.isValid() && end.isValid())
  {
    model::FileLoc fileLoc;
    _fileLocUtil.setRange(start, end, fileLoc.range);
    fileLoc.file = _ctx.srcMgr.getFile(
      _fileLocUtil.getFilePath(start));

    const std::string& type = fileLoc.file.load()->type;
    if (type != model::File::DIRECTORY_TYPE && type != _cppSourceType)
    {
      fileLoc.file->type = _cppSourceType;
      _ctx.srcMgr.updateFile(*fileLoc.file);
    }
    astNode->location = fileLoc;
  }
}

bool PPMacroCallback::isBuiltInMacro(const clang::MacroInfo* mi_) const
{
  clang::PresumedLoc presLoc = getPresumedLocation(
    _pp.getSourceManager(), mi_->getDefinitionLoc());

  std::string fileName = presLoc.getFilename();

  return fileName == "<built-in>" || fileName == "<command line>";
}

std::string PPMacroCallback::getMangledName(const clang::MacroInfo *mi_)
{
  std::string mangledName;

  clang::SourceManager& srcMan = _pp.getSourceManager();
  clang::PresumedLoc presLoc =
    getPresumedLocation(srcMan, mi_->getDefinitionLoc());

  if (!presLoc.getFilename())
  {
    return std::string();
  }

  std::string fileName = presLoc.getFilename();

  if (isBuiltInMacro(mi_))
     return
       std::to_string(presLoc.getLine()) + ":" +
       std::to_string(presLoc.getColumn())
        + ":" + fileName;

  model::FilePtr file = _ctx.srcMgr.getFile(fileName);
  if (!file)
    return std::to_string(presLoc.getLine())   + ":" +
           std::to_string(presLoc.getColumn()) + ":" + fileName;
  return
    std::to_string(presLoc.getLine())   + ":" +
    std::to_string(presLoc.getColumn()) + ":" +
    std::to_string(file->id);
}

bool PPMacroCallback::insertToCache(const model::CppAstNodePtr node_)
{
  static std::mutex cacheMutex;
  std::lock_guard<std::mutex> guard(cacheMutex);

  return _mangledNameCache.insert(
    std::make_pair(node_->id, node_->mangledNameHash)).second;
}

} // parser
} // cc
