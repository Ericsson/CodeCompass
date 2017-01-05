#ifndef CXXPARSER_DOCUMENTATIONCOMMENTCOLLECTOR_H
#define CXXPARSER_DOCUMENTATIONCOMMENTCOLLECTOR_H

#include <map>
#include <set>
#include <iostream>
#include <stack>
#include <typeinfo>
#include <functional>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Specifiers.h>

#include <clang/AST/Decl.h>
#include <clang/AST/Comment.h>

#include <model/workspace.h>
#include <model/comment/doccomment.h>
#include <model/comment/doccomment-odb.hxx>

#include "cxxparsesession.h"
#include "documentationcommentformatter.h"

namespace cc
{
namespace parser
{

class DocumentationCommentCollector : public clang::RecursiveASTVisitor<DocumentationCommentCollector>
{
public:
  DocumentationCommentCollector(
      std::shared_ptr<model::Workspace> w_,
      CxxParseSession& session_
      ) :
    _Ctx(session_.astContext),
    _w(w_),
    _clangSrcMgr(session_.astContext.getSourceManager()),
    _session(session_),
    _clang2our(session_.clang2our),
    _newNodes(session_.newNodes) { }


  bool VisitNamedDecl(const clang::NamedDecl *decl)
  {
    if (!decl)
    {
      SLog(util::ERROR) << "The visitor got nullpointer!!!";
      return true;
    }

    if (_newNodes.find(decl) == _newNodes.end())
    {
      return true;
    }

    clang::comments::FullComment *fc = decl->getASTContext().getCommentForDecl(
      decl, nullptr/* PP */);
    if (!fc) return true;

    const model::CppAstNodePtr& astNode = _clang2our.at(decl);

    std::map<std::string, unsigned long long> paramMap;
    const clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(decl);
    if (FD) FunctionParams(FD, paramMap);

    DocumentationCommentFormatter dcFmt;
    std::string commentPrettyText = dcFmt.format(
      _clangSrcMgr, fc, paramMap, _session);

    model::DocCommentPtr pc(new model::DocComment);
    pc->contentHTML = commentPrettyText;
    pc->contentHash = util::fnvHash(pc->contentHTML);
    pc->mangledNameHash = astNode->mangledNameHash;
    _docComments.insert(std::make_pair(
      std::make_pair(pc->mangledNameHash, pc->contentHash), pc));

    return true;
  }

  ~DocumentationCommentCollector()
  {
    util::OdbTransaction trans(*_w->getDb());
    trans([this]()
    {
      for (auto cmt: _docComments)
      {
        _w->getDb()->persist(*(cmt.second));
      }
    });
  }

private:
  void FunctionParams(const clang::FunctionDecl* decl, std::map<std::string, unsigned long long>& params_)
  {
      for( auto it = decl->param_begin(); it != decl->param_end(); ++it)
      {
        auto mn = _clang2our.find(static_cast<const void*>((*it)));
        if (mn != _clang2our.end())
        {
          params_.insert(std::pair<std::string, unsigned long long>
                         ((*it)->getNameAsString(), mn->second->id));
        }
      }
  }

  clang::ASTContext& _Ctx;
  std::shared_ptr<model::Workspace> _w;
  const clang::SourceManager& _clangSrcMgr;
  CxxParseSession& _session;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>& _newNodes;

  std::map<
    std::pair<unsigned long long, unsigned long long>,
    model::DocCommentPtr> _docComments;


};

} // parser
} // cc

#endif // CXXPARSER_DOCUMENTATIONCOMMENTCOLLECTOR_H
