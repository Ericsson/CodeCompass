#ifndef CC_PARSER_ASSIGNMENTCOLLECTOR_H
#define CC_PARSER_ASSIGNMENTCOLLECTOR_H

#include <iostream>
#include <unordered_set>
#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cpprelation.h>
#include <model/cpprelation-odb.hxx>
#include <parser/parsercontext.h>

#include <util/odbtransaction.h>
#include "symbolhelper.h"
#include "manglednamecache.h"
#include "refcollector.h"

namespace cc
{
namespace parser
{

class AssignmentCollector
  : public clang::RecursiveASTVisitor<AssignmentCollector>
{
public:
  AssignmentCollector(
    ParserContext& ctx_,
    clang::ASTContext&,
    MangledNameCache& mangledNameCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
    : _ctx(ctx_),
      _mangledNameCache(mangledNameCache_),
      _clangToAstNodeId(clangToAstNodeId_)
  {
  }

  ~AssignmentCollector()
  {
    (util::OdbTransaction(_ctx.db))([this]{
      for (model::CppRelation& relation : _relations)
        _ctx.db->persist(relation);
    });
  }

  bool VisitBinaryOperator(clang::BinaryOperator* bop_)
  {
    if (bop_->isAssignmentOp() ||
        bop_->isCompoundAssignmentOp() ||
        bop_->isShiftAssignOp())
      makeAssignRels(bop_->getLHS(), bop_->getRHS());

    return true;
  }

  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* ce_)
  {
    clang::FunctionDecl* callee = ce_->getDirectCallee();

    if (!callee)
      return true;

    int x = llvm::isa<clang::CXXMethodDecl>(callee) ? 1 : 0;
    for (std::size_t i = 0;
         i < callee->getNumParams() && i + x < ce_->getNumArgs();
         ++i)
      makeAssignRels(callee->getParamDecl(i), ce_->getArg(i + x));

    return true;
  }

  bool VisitCallExpr(clang::CallExpr* ce_)
  {
    clang::FunctionDecl* callee = ce_->getDirectCallee();

    if (!callee)
      return true;

    for (std::size_t i = 0;
         i < callee->getNumParams() && i < ce_->getNumArgs();
         ++i)
      makeAssignRels(callee->getParamDecl(i), ce_->getArg(i));

    return true;
  }

  bool VisitVarDecl(clang::VarDecl* vd_)
  {
    makeAssignRels(vd_, vd_->getInit());
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr* ce_)
  {
    clang::CXXConstructorDecl* callee = ce_->getConstructor();

    if (!callee)
      return true;

    for (std::size_t i = 0;
         i < callee->getNumParams() && i < ce_->getNumArgs();
         ++i)
      makeAssignRels(callee->getParamDecl(i), ce_->getArg(i));

    return true;
  }

  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl* cd_)
  {
    for (auto it = cd_->init_begin(), e = cd_->init_end(); it != e; ++it)
    {
      const clang::CXXCtorInitializer* ci = *it;
      if (!ci->getMember() || ci->getSourceOrder() == -1)
        continue;

      makeAssignRels(ci->getMember(), ci->getInit());
    }

    return true;
  }

private:
  template <typename T>
  std::unordered_set<const void*> collect(T* s_)
  {
    std::unordered_set<const void*> ret;

    RefCollector rc(ret, isFunction);
    rc.collect(s_);

    return ret;
  }

  template <typename T1, typename T2>
  void makeAssignRels(T1* written, T2* read)
  {
    auto lhs = collect(written);
    auto rhs = collect(read);

    for (auto w : collect(written))
      for (auto r : collect(read))
      {
        if (w == r)
          continue;

        auto itW = _clangToAstNodeId.find(w);
        auto itR = _clangToAstNodeId.find(r);

        if (itW == _clangToAstNodeId.end() || itR == _clangToAstNodeId.end())
          continue;

        model::CppRelation relation;
        relation.kind = model::CppRelation::Kind::Assign;
        relation.lhs = _mangledNameCache.at(itW->second);
        relation.rhs = _mangledNameCache.at(itR->second);
        _relations.push_back(std::move(relation));
      }
  }

  ParserContext& _ctx;
  MangledNameCache& _mangledNameCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;
  std::vector<model::CppRelation> _relations;
};

}
}

#endif
