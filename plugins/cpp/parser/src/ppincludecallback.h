#ifndef CC_PARSER_PPINCLUDECALLBACK_H
#define CC_PARSER_PPINCLUDECALLBACK_H

#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/ASTContext.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppheaderinclusion.h>
#include <model/cppheaderinclusion-odb.hxx>

#include <parser/parsercontext.h>

#include <util/logutil.h>

#include "entitycache.h"

namespace cc
{
namespace parser
{

class PPIncludeCallback : public clang::PPCallbacks
{
public:
  PPIncludeCallback(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    EntityCache& entityCache_,
    clang::Preprocessor& pp_);

  ~PPIncludeCallback();

  virtual void InclusionDirective(
    clang::SourceLocation HashLoc,
    const clang::Token &IncludeTok,
    clang::StringRef FileName,
    bool IsAngled,
    clang::CharSourceRange FilenameRange,
    clang::Optional<clang::FileEntryRef> File,
    clang::StringRef SearchPath,
    clang::StringRef RelativePath,
    const clang::Module *Imported,
    clang::SrcMgr::CharacteristicKind FileType) override;

private:
  /**
   * This function creates an AST Node from a file.
   */
  model::CppAstNodePtr createFileAstNode(
    const model::FilePtr& file_,
    const clang::SourceRange& srcRange_);

  ParserContext& _ctx;
  const std::string _cppSourceType;
  const clang::SourceManager& _clangSrcMgr;
  FileLocUtil _fileLocUtil;
  EntityCache& _entityCache;

  std::vector<model::CppAstNodePtr>         _astNodes;
  std::vector<model::CppHeaderInclusionPtr> _headerIncs;
};

} // parser
} // cc

#endif // CC_PARSER_PPINCLUDECALLBACK_H
