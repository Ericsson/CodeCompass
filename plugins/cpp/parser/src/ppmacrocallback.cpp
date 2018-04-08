#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include "filelocutil.h"
#include "ppmacrocallback.h"

namespace cc
{
namespace parser
{

PPMacroCallback::PPMacroCallback(
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  MangledNameCache& mangledNameCache_,
  clang::Preprocessor& pp_) :
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
    util::persistAll(_astNodes, _ctx.db);
    util::persistAll(_macros, _ctx.db);
    util::persistAll(_macrosExpansion, _ctx.db);
  });
}

void PPMacroCallback::MacroExpands(
  const clang::Token& macroNameTok_,
  const clang::MacroDefinition& md_,
  clang::SourceRange range_,
  const clang::MacroArgs*)
{
  if (_disabled)
    return;

  clang::SourceLocation loc = macroNameTok_.getLocation();
  if (loc.isInvalid() || !loc.isFileID())
    return;

  const clang::MacroInfo* mi = md_.getMacroInfo();

  if (!mi || getMangledName(mi).empty())
    return;

  const char* begin = _clangSrcMgr.getCharacterData(range_.getBegin());
  int len = _clangSrcMgr.getCharacterData(range_.getEnd()) - begin +
    clang::Lexer::MeasureTokenLength(
      range_.getEnd(), _clangSrcMgr, _pp.getLangOpts());

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
  TmpDiags.setSourceManager(&_clangSrcMgr);

  _disabled = true;
  clang::DiagnosticsEngine *OldDiags = &_pp.getDiagnostics();
  _pp.setDiagnostics(TmpDiags);
  _pp.EnterTokenStream(tokens, false);

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
  _disabled = false;

  model::CppAstNodePtr astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::Usage;
  addFileLoc(astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());
  astNode->id = model::createIdentifier(*astNode);

  if (_mangledNameCache.insert(*astNode))
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

  if (_mangledNameCache.insert(*astNode))
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
  const clang::MacroDefinition& md_,
  const clang::MacroDirective* undef_)
{
  const clang::MacroInfo* mi = md_.getMacroInfo();

  if (!mi || isBuiltInMacro(mi) || getMangledName(mi).empty())
    return;

  auto astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::UnDefinition;
  addFileLoc(astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());
  astNode->id = model::createIdentifier(*astNode);

  if (_mangledNameCache.insert(*astNode))
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
  model::CppAstNodePtr& astNode_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_)
{
  if (start_.isInvalid() || end_.isInvalid())
    return;

  model::FileLoc fileLoc;
  _fileLocUtil.setRange(start_, end_, fileLoc.range);
  fileLoc.file = _ctx.srcMgr.getFile(_fileLocUtil.getFilePath(start_));

  const std::string& type = fileLoc.file.load()->type;
  if (type != model::File::DIRECTORY_TYPE && type != _cppSourceType)
  {
    fileLoc.file->type = _cppSourceType;
    _ctx.srcMgr.updateFile(*fileLoc.file);
  }

  astNode_->location = fileLoc;
}

bool PPMacroCallback::isBuiltInMacro(const clang::MacroInfo* mi_) const
{
  std::string fileName = _clangSrcMgr.getPresumedLoc(
    _clangSrcMgr.getExpansionLoc(mi_->getDefinitionLoc())).getFilename();

  return fileName == "<built-in>" || fileName == "<command line>";
}

std::string PPMacroCallback::getMangledName(const clang::MacroInfo* mi_)
{
  clang::PresumedLoc presLoc = _clangSrcMgr.getPresumedLoc(
    _clangSrcMgr.getExpansionLoc(mi_->getDefinitionLoc()));

  const char* fileName = presLoc.getFilename();

  if (!fileName)
    return std::string();

  std::string locStr
     = std::to_string(presLoc.getLine())   + ":" +
       std::to_string(presLoc.getColumn()) + ":";

  return locStr
    + (isBuiltInMacro(mi_)
    ? fileName
    : std::to_string(_ctx.srcMgr.getFile(fileName)->id));
}

} // parser
} // cc
