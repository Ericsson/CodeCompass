#ifndef CXXPARSER_REFCOLLECTOR_H
#define CXXPARSER_REFCOLLECTOR_H

#include <functional>
#include <unordered_set>

#include <clang/AST/RecursiveASTVisitor.h>

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include "tracer.h"

namespace cc
{
namespace parser
{

class RefCollector : public clang::RecursiveASTVisitor<RefCollector>
{
public:
  typedef std::function<bool(const clang::Type*)> RefTester;

  RefCollector(std::unordered_set<const void*>& collected_) :
      collected(collected_)
  {

  }

  RefCollector(std::unordered_set<const void*>& collected_,
    RefTester refTester) :
      collected(collected_),
      refTester(refTester)
  {

  }

  void collect(clang::Stmt* start)
  {
    startStmt = start;
    this->TraverseStmt(startStmt);
  }

  void collect(clang::Decl* decl)
  {
    startDecl = decl;
    this->TraverseDecl(startDecl);
  }

  bool TraverseCXXConstructExpr(clang::CXXConstructExpr *ce_)
  {
    bool didISwitchItOff = false;

    if (startStmt != ce_ && shouldCollect)
    {
      shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCXXConstructExpr(ce_);

    if (didISwitchItOff)
    {
      shouldCollect = true;
    }

    return true;
  }

  bool TraverseCallExpr(clang::CallExpr* ce_)
  {
    bool didISwitchItOff = false;

    if (startStmt != ce_ && shouldCollect)
    {
      shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCallExpr(ce_);

    if (didISwitchItOff)
    {
      shouldCollect = true;
    }

    return true;
  }

  bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* ce_)
  {
    bool didISwitchItOff = false;

    if (startStmt != ce_ && shouldCollect)
    {
      shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCXXMemberCallExpr(ce_);

    if (didISwitchItOff)
    {
      shouldCollect = true;
    }

    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* d_)
  {
    tryCollectStmt(d_);
    return true;
  }

  bool VisitMemberExpr( clang::MemberExpr* expr)
  {
    tryCollectStmt(expr);
    return true;
  }

  bool VisitValueDecl(clang::ValueDecl* d_)
  {
    tryCollectStmt(d_);
    return true;
  }

private:
  template <typename T>
  void tryCollectStmt(T* t_)
  {
    if (!shouldCollect)
      return;

    if (!refTester || refTester(t_->getType().getTypePtrOrNull()))
    {
      collected.insert(t_);
    }
  }

  std::string getQualifiedName(clang::DeclRefExpr* d_)
  {
    return d_->getDecl()->getQualifiedNameAsString();
  }

  std::string getQualifiedName(clang::MemberExpr* e_)
  {
    return e_->getMemberDecl()->getQualifiedNameAsString();
  }

  std::string getQualifiedName(clang::ValueDecl* d_)
  {
    return d_->getQualifiedNameAsString();
  }

  union
  {
    clang::Stmt* startStmt;
    clang::Decl* startDecl;
  };

  std::unordered_set<const void*>& collected;
  RefTester                        refTester;

  bool                             shouldCollect = true;
};

} // parser
} // cc

#endif //CXXPARSER_REFCOLLECTOR_H
