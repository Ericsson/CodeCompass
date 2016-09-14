/*
 * macrohandler.cpp
 *
 *  Created on: Jul 4, 2013
 *      Author: ezoltbo
 */

#include <stack>

#include "ccppcallbacks.h"

#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/IdentifierTable.h>

#include "util/streamlog.h"
#include "util/util.h"

#include "model/cxx/cppastnode.h"
#include "model/cxx/cppastnode-odb.hxx"
#include "model/cxx/cppmacro.h"
#include "model/cxx/cppmacro-odb.hxx"
#include "model/cxx/cppmacroexpansion.h"
#include "model/cxx/cppmacroexpansion-odb.hxx"
#include "model/cxx/cppheaderinclusion.h"
#include "model/cxx/cppheaderinclusion-odb.hxx"

#include <model/workspace.h>

#include "filelocutil.h"

namespace cc
{
namespace parser
{

namespace
{

clang::PresumedLoc getPresumedLocation(
  const clang::SourceManager &sm,
  clang::SourceLocation loc)
{
  auto expLoc = sm.getExpansionLoc(loc);
  return sm.getPresumedLoc(expLoc);
}

void getBuffer(
  const clang::SourceManager& sm_,
  clang::SourceLocation loc_,
  const char*& buffer_,
  std::size_t& bufferSize_)
{
  clang::FileID fid = sm_.getFileID(loc_);
  if (fid.isInvalid())
  {
    return;
  }

  bool buffInvalid = false;
  llvm::StringRef buff = sm_.getBufferData(fid, &buffInvalid);
  if (buffInvalid)
  {
    return;
  }

  buffer_ = buff.data();
  bufferSize_ = buff.size();
}

void getBuffer(
  clang::SourceManager& sm_,
  const clang::FileEntry* file_,
  const char*& buffer_,
  std::size_t& bufferSize_)
{
  if (!file_)
  {
    return;
  }

  bool buffInvalid = false;
  llvm::MemoryBuffer* buffPtr = sm_.getMemoryBufferForFile(file_, &buffInvalid);
  if (!buffPtr || buffInvalid)
  {
    return;
  }

  llvm::StringRef buff = buffPtr->getBuffer();
  buffer_ = buff.data();
  bufferSize_ = buff.size();
}

} // anonymous namespace

CcPPCallbacks::~CcPPCallbacks()
{
  astPersister.persistAstNodes(_astNodes);
  astPersister.persistEntities(_macros);

  util::OdbTransaction trans(*w->getDb());
  trans([this]()
  {
    for (auto& exp : _macrosExpansion)
    {
      auto it = std::find(_astNodes.begin(), _astNodes.end(), exp->astNodePtr);
      if (it != _astNodes.end())
      {
        w->getDb()->persist(*exp);
      }
    }

    for (auto& hdr : _headerIncs)
    {
      w->getDb()->persist(*hdr);
    }
  });
}

void CcPPCallbacks::addFileLoc(model::CppAstNode& astNode,
                const clang::SourceLocation& start,
                const clang::SourceLocation& end)
{
  if(start.isValid() && end.isValid())
  {
    FileLocUtil flu(ccSrcMgr, pp.getSourceManager());

    model::FileLoc fileLoc;
    if (flu.setInfo(start, end, fileLoc))
    {
      astNode.location = fileLoc;
    }
  }
}


bool CcPPCallbacks::isBuiltInMacro(const clang::MacroInfo* mi_) const
{
  auto presLoc = getPresumedLocation(
    pp.getSourceManager(),
    mi_->getDefinitionLoc());

  std::string fileName = presLoc.getFilename();

  return fileName == "<built-in>" || fileName == "<command line>";
}

void CcPPCallbacks::MacroExpands(
  const clang::Token& macroNameTok_,
  const clang::MacroDirective* md_,
  clang::SourceRange range_,
  const clang::MacroArgs* args_)
{
  MacroExpands(macroNameTok_, md_->getMacroInfo(), range_);
}

void CcPPCallbacks::MacroExpands(
  const clang::Token& macroNameTok_,
  const clang::MacroInfo* mi_,
  clang::SourceRange range_)
{
  if (disabled)
      return;

  clang::SourceLocation loc = macroNameTok_.getLocation();
  if (!loc.isValid() || !loc.isFileID())
      return;

  auto &srcMgr = pp.getSourceManager();

  auto B = getPresumedLocation(srcMgr, range_.getBegin());
  auto E = getPresumedLocation(srcMgr, range_.getEnd());

  const char *begin = srcMgr.getCharacterData(range_.getBegin());
  int len = srcMgr.getCharacterData(range_.getEnd()) - begin;
  len += clang::Lexer::MeasureTokenLength(range_.getEnd(), srcMgr, pp.getLangOpts());

  std::string copy(begin, len);
  begin = copy.c_str();
  clang::Lexer lex(loc, pp.getLangOpts(), begin, begin, begin + len);
  std::vector<clang::Token> tokens;

  //Lousely based on code from clang::html::HighlightMacros

  // Lex all the tokens in raw mode, to avoid entering #includes or expanding
  // macros.
  clang::Token tok;
  do {
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
          pp.LookUpIdentifierInfo(tok);

      tokens.push_back(tok);

  } while(!tok.is(clang::tok::eof));

  // Temporarily change the diagnostics object so that we ignore any generated
  // diagnostics from this pass.
  clang::DiagnosticsEngine TmpDiags(
    pp.getDiagnostics().getDiagnosticIDs(),
    &pp.getDiagnostics().getDiagnosticOptions(),
    new clang::IgnoringDiagConsumer);
  TmpDiags.setSourceManager(&pp.getSourceManager());

  disabled = true;
  clang::DiagnosticsEngine *OldDiags = &pp.getDiagnostics();
  pp.setDiagnostics(TmpDiags);

  pp.EnterTokenStream(tokens.data(), tokens.size(), false, false);

  std::string expansion;
  pp.Lex(tok);
  while(tok.isNot(clang::tok::eof)) {
      // If the tokens were already space separated, or if they must be to avoid
      // them being implicitly pasted, add a space between them.
      if (tok.hasLeadingSpace())
          expansion += ' ';
         // ConcatInfo.AvoidConcat(PrevPrevTok, PrevTok, Tok)) //FIXME
      // Escape any special characters in the token text.
      expansion += pp.getSpelling(tok);
      pp.Lex(tok);
  }

  pp.setDiagnostics(*OldDiags);
  disabled = false;

  auto mangledName = getMangledName(mi_);
  if (mangledName.empty())
    return;

  auto astNode = createMacroAstNode(macroNameTok_, mi_);
  astNode->astType = model::CppAstNode::AstType::Usage;
  addFileLoc(*astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());

  _astNodes.push_back(astNode);

  model::CppMacroExpansionPtr mExp(new model::CppMacroExpansion);
  mExp->astNodePtr = astNode;
  mExp->expansion = expansion;

  _macrosExpansion.push_back(mExp);
}

void CcPPCallbacks::MacroDefined(
  const clang::Token& macroNameTok_,
  const clang::MacroDirective* md_)
{
  auto mi = md_->getMacroInfo();
  if (!mi)
  {
    return;
  }

  if (isBuiltInMacro(mi))
    return;

  auto mangledName = getMangledName(mi);
  if (mangledName.empty())
    return;

  macroInfo.insert(std::make_pair(
        macroNameTok_.getIdentifierInfo()->getName().str(), mi));

  auto astNode = createMacroAstNode(macroNameTok_, mi);
  astNode->astType = model::CppAstNode::AstType::Definition;
  addFileLoc(*astNode, mi->getDefinitionLoc(), mi->getDefinitionEndLoc());

  // create CppMacro entity
  _astNodes.push_back(astNode);

  auto macro = std::make_shared<model::CppMacro>();
  macro->astNodeId = astNode->id;
  macro->mangledNameHash = astNode->mangledNameHash;
  macro->name = astNode->astValue;
  macro->qualifiedName = astNode->astValue;
  _macros.push_back(macro);
}

void CcPPCallbacks::MacroUndefined(
  const clang::Token& macroNameTok_,
  const clang::MacroDirective* md_)
{
  macroInfo.erase(macroNameTok_.getName());

  if (!md_)
  {
    // Shit happens
    return;
  }

  auto mangledName = getMangledName(md_->getMacroInfo());
  if (mangledName.empty())
    return;

  auto astNode = createMacroAstNode(macroNameTok_, md_->getMacroInfo());
  astNode->astType = model::CppAstNode::AstType::UnDefinition;
  addFileLoc(*astNode, macroNameTok_.getLocation(), macroNameTok_.getLastLoc());

  _astNodes.push_back(astNode);
}

model::CppAstNodePtr CcPPCallbacks::createMacroAstNode(
  const clang::Token& macroNameTok_,
  const clang::MacroInfo* mi_)
{
  model::CppAstNodePtr astNode(new model::CppAstNode());

  astNode->astValue = macroNameTok_.getIdentifierInfo()->getName().str();
  astNode->mangledName = getMangledName(mi_);
  astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
  astNode->symbolType = model::CppAstNode::SymbolType::Macro;

  return astNode;
}

model::CppAstNodePtr CcPPCallbacks::createFileAstNode(
  const model::FilePtr& file,
  const clang::SourceRange& srcRange)
{
  model::CppAstNodePtr astNode(new model::CppAstNode());

  astNode->astValue = std::to_string(file->id);
  astNode->mangledName = file->path;
  astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
  astNode->symbolType = model::CppAstNode::SymbolType::File;
  astNode->astType = model::CppAstNode::AstType::Usage;

  addFileLoc(*astNode, srcRange.getBegin(), srcRange.getEnd());

  return astNode;
}

void CcPPCallbacks::InclusionDirective(
  clang::SourceLocation HashLoc,
  const clang::Token &IncludeTok,
  clang::StringRef FileName,
  bool IsAngled,
  clang::CharSourceRange FilenameRange,
  const clang::FileEntry *File,
  clang::StringRef SearchPath,
  clang::StringRef RelativePath,
  const clang::Module *Imported)
{
  if (SearchPath.empty())
    return;

  clang::SourceManager& srcMgr = pp.getSourceManager();
  auto presLoc = getPresumedLocation(pp.getSourceManager(), HashLoc);

  std::string includedPath = SearchPath.str() + '/' + FileName.str();
  const char* includedBuff = nullptr;
  std::size_t includedBuffSize = 0;
  std::string includerPath = presLoc.getFilename();
  const char* includerBuff = nullptr;
  std::size_t includerBuffSize = 0;

  getBuffer(srcMgr, HashLoc, includerBuff, includerBuffSize);
  getBuffer(srcMgr, File, includedBuff, includedBuffSize);

  model::FilePtr includer, included;
  if (!ccSrcMgr.getCreateFile(
        includerPath,
        includer,
        SourceManager::Defaults,
        includerBuff, includerBuffSize) ||
      !ccSrcMgr.getCreateFile(
        includedPath,
        included,
        SourceManager::Defaults,
        includedBuff, includedBuffSize))
  {
    return;
  }

  model::CppHeaderInclusionPtr inclusion(new model::CppHeaderInclusion);
  inclusion->includer = includer;
  inclusion->included = included;
  inclusion->context = targetFile;

  auto fileAst = createFileAstNode(included, FilenameRange.getAsRange());
  _astNodes.push_back(fileAst);
  _headerIncs.push_back(inclusion);
}

std::string CcPPCallbacks::getMangledName(const clang::MacroInfo *mi_)
{
  std::string mangledName;

  clang::SourceManager& srcMan = pp.getSourceManager();
  auto presLoc = getPresumedLocation(srcMan, mi_->getDefinitionLoc());

  if (!presLoc.getFilename())
  {
    return std::string();
  }

  std::string fileName = presLoc.getFilename();

  if (isBuiltInMacro(mi_))
  {
     return
       std::to_string(presLoc.getLine()) + ":" +
       std::to_string(presLoc.getColumn())
        + ":" + fileName;
  }

  model::FilePtr file;
  const char*    fileBuff = nullptr;
  std::size_t    fileBuffSize = 0;

  getBuffer(srcMan, presLoc.getIncludeLoc(), fileBuff, fileBuffSize);
  if (!ccSrcMgr.getCreateFile(
        fileName,
        file,
        SourceManager::Defaults,
        fileBuff, fileBuffSize))
  {
    mangledName =
      std::to_string(presLoc.getLine()) + ":" +
      std::to_string(presLoc.getColumn()) + ":" + fileName;
  }
  else
  {
    mangledName =
      std::to_string(presLoc.getLine()) + ":" +
      std::to_string(presLoc.getColumn())+ ":" +
      std::to_string(file.object_id());
  }

  return mangledName;
}

} // parser
} // cc

