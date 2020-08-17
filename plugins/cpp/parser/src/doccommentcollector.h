#ifndef CC_PARSER_DOCCOMMENTCOLLECTOR_H
#define CC_PARSER_DOCCOMMENTCOLLECTOR_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Comment.h>

#include <clang/Basic/Specifiers.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Index/CommentToXML.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Refactoring.h>

#include <model/cppdoccomment.h>
#include <model/cppdoccomment-odb.hxx>

#include "doccommentformatter.h"
#include "manglednamecache.h"

namespace cc
{
namespace parser
{

class DocCommentCollector :
  public clang::RecursiveASTVisitor<DocCommentCollector>
{
public:
  DocCommentCollector(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
      : _ctx(ctx_),
        _astContext(astContext_),
        _clangSrcMgr(astContext_.getSourceManager()),
        _mangledNameCache(mangledNameCache_),
        _clangToAstNodeId(clangToAstNodeId_)
  {
  }

  bool VisitNamedDecl(const clang::NamedDecl *decl)
  {
    if (!decl)
      return true;

    auto it = _clangToAstNodeId.find(decl);
    if (it == _clangToAstNodeId.end())
      return true;

    clang::comments::FullComment* fc =
      decl->getASTContext().getCommentForDecl(decl, nullptr);

    if (!fc) return true;

    DocCommentFormatter dcFmt;

    model::CppDocCommentPtr pc(new model::CppDocComment);
    pc->content = dcFmt.format(fc, _astContext);
    pc->contentHash = util::fnvHash(pc->content);
    pc->mangledNameHash = _mangledNameCache.at(it->second);
    _docComments.insert(std::make_pair(
      std::make_pair(pc->mangledNameHash, pc->contentHash), pc));

    return true;
  }

  ~DocCommentCollector()
  {
    (util::OdbTransaction(_ctx.db))([this]{
      for (auto cmt : _docComments)
      {
        _ctx.db->persist(*(cmt.second));
      }
    });
  }

private:
  std::map<std::pair<unsigned long long, unsigned long long>,
    model::CppDocCommentPtr> _docComments;

  ParserContext& _ctx;
  const clang::ASTContext& _astContext;
  const clang::SourceManager& _clangSrcMgr;
  MangledNameCache& _mangledNameCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;
};

} // parser
} // cc

#endif // CC_PARSER_DOCCOMMENTCOLLECTOR_H
