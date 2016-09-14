/*
 * macrohandler.h
 *
 *  Created on: Jul 4, 2013
 *      Author: ezoltbo
 */

#ifndef CODECOMPASS_MACROHANDLER_H_
#define CODECOMPASS_MACROHANDLER_H_

#include <clang/Frontend/FrontendActions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PPCallbacks.h>

#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "parser/sourcemanager.h"
#include "model/cxx/cppastnode.h"
#include "model/cxx/cppmacro.h"
#include "model/cxx/cppmacroexpansion.h"
#include "model/cxx/cppheaderinclusion.h"
#include "symbolhelper.h"

#include "cxxparser/cxxastpersister.h"

namespace cc
{
namespace parser
{

class CcPPCallbacks : public clang::PPCallbacks
{
public:
  CcPPCallbacks(clang::Preprocessor& pp, clang::ASTContext &ctx,
    SourceManager& srcMgr,
    std::shared_ptr<model::Workspace> w,
    model::FilePtr targetFile,
    CxxAstPersister &astPersister)
    : pp(pp), ccSrcMgr(srcMgr), w(w), symbolHelper(ctx),
      targetFile(targetFile), astPersister(astPersister)
  {}

  ~CcPPCallbacks();

  virtual void MacroExpands(
    const clang::Token& macroNameTok_,
    const clang::MacroDirective* md_,
    clang::SourceRange range_,
    const clang::MacroArgs* args_) override;

  virtual void MacroDefined(
    const clang::Token& macroNameTok_,
    const clang::MacroDirective* md_) override;

  virtual void MacroUndefined(
    const clang::Token& macroNameTok_,
    const clang::MacroDirective* md_) override;

  virtual void InclusionDirective(clang::SourceLocation HashLoc,
                                  const clang::Token &IncludeTok,
                                  clang::StringRef FileName,
                                  bool IsAngled,
                                  clang::CharSourceRange FilenameRange,
                                  const clang::FileEntry *File,
                                  clang::StringRef SearchPath,
                                  clang::StringRef RelativePath,
                                  const clang::Module *Imported) override;
private:
  void MacroExpands(
    const clang::Token& macroNameTok_,
    const clang::MacroInfo* mi_,
    clang::SourceRange range_);

  std::string getMangledName(const clang::MacroInfo *MI);

  model::CppAstNodePtr createMacroAstNode(const clang::Token& macroNameTok_,
                                          const clang::MacroInfo* mi_);

  model::CppAstNodePtr createFileAstNode(const model::FilePtr& file,
                                         const clang::SourceRange& srcRange);

  void addFileLoc(model::CppAstNode& astNode,
                  const clang::SourceLocation& start,
                  const clang::SourceLocation& end);

  bool isBuiltInMacro(const clang::MacroInfo* mi_) const;

  clang::Preprocessor &pp;
  SourceManager &ccSrcMgr;
  std::shared_ptr<model::Workspace> w;
  SymbolHelper symbolHelper;
  model::FilePtr targetFile;
  CxxAstPersister &astPersister;
  std::unordered_map<std::string, const clang::MacroInfo*> macroInfo;

  bool disabled = false;

  std::vector<model::CppAstNodePtr> _astNodes;
  std::vector<model::CppMacroPtr> _macros;
  std::vector<model::CppMacroExpansionPtr> _macrosExpansion;
  std::vector<model::CppHeaderInclusionPtr> _headerIncs;
};

} // parser
} // cc
#endif /* CODECOMPASS_MACROHANDLER_H_ */
