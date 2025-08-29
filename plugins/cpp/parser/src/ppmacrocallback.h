#ifndef CC_PARSER_PPMACROCALLBACK_H
#define CC_PARSER_PPMACROCALLBACK_H

#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/ASTContext.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppmacro.h>
#include <model/cppmacro-odb.hxx>
#include <model/cppmacroexpansion.h>
#include <model/cppmacroexpansion-odb.hxx>

#include <parser/parsercontext.h>

#include <util/logutil.h>

#include "entitycache.h"

namespace cc
{
namespace parser
{

class PPMacroCallback : public clang::PPCallbacks
{
public:
  PPMacroCallback(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    EntityCache& entityCache_,
    clang::Preprocessor& pp_);

  ~PPMacroCallback();

  /*virtual void MacroExpands(
    const clang::Token& macroNameTok_,
    const clang::MacroDefinition& md_,
    clang::SourceRange range_,
    const clang::MacroArgs* args_) override;*/

  virtual void MacroDefined(
    const clang::Token& macroNameTok_,
    const clang::MacroDirective* md_) override;

  virtual void MacroUndefined(
    const clang::Token& macroNameTok_,
    const clang::MacroDefinition& md_,
    const clang::MacroDirective* undef_) override;

private:

  /**
   * This function creates an AST Node for a macro.
   */
  model::CppAstNodePtr createMacroAstNode(
    const clang::Token& macroNameTok_,
    const clang::MacroInfo* mi_);

  /**
   * This function tells that a macro is a built-in macro.
   */
  bool isBuiltInMacro(const clang::MacroInfo* mi_) const;

  /**
   * This function returns a unique identifier of the macro.
   *
   * TODO: In Clang there is a function named generateUSRForMacro(), but we're
   * not using that in this implementation. That function gets a
   * MacroDefinitionRecord as parameter, but here we have a MacroInfo.
   */
  std::string getUSR(const clang::MacroInfo *MI);

  /**
   * This function add fill the file location of the AST node.
   */
  void addFileLoc(
    model::CppAstNodePtr& astNode,
    const clang::SourceLocation& start,
    const clang::SourceLocation& end);

  ParserContext& _ctx;
  clang::Preprocessor& _pp;
  const std::string _cppSourceType;
  clang::SourceManager& _clangSrcMgr;
  FileLocUtil _fileLocUtil;

  bool _disabled = false;

  EntityCache& _entityCache;
  std::vector<model::CppAstNodePtr>        _astNodes;
  std::vector<model::CppMacroPtr>          _macros;
  std::vector<model::CppMacroExpansionPtr> _macrosExpansion;

};

} // parser
} // cc

#endif // CC_PARSER_PPMACROCALLBACK_H
