#ifndef CXXPARSER_ASSIGNMENTCOLLECTOR_H
#define CXXPARSER_ASSIGNMENTCOLLECTOR_H

#include <map>
#include <set>
#include <iostream>
#include <stack>
#include <typeinfo>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/Specifiers.h>

#include <cxxparser/cxxparser.h>

#include <model/workspace.h>
#include <model/cxx/cpprelation.h>
#include <model/cxx/cpprelation-odb.hxx>

#include <util/streamlog.h>

#include "cxxparsesession.h"
#include "refcollector.h"

namespace cc
{
namespace parser
{

class AssignmentCollector : public clang::RecursiveASTVisitor<AssignmentCollector>
{
public:
  AssignmentCollector(std::shared_ptr<model::Workspace> w_,
                    CxxParseSession& session_
                   ) :
    _w(w_), _clang2our(session_.clang2our),
    _newNodes(session_.newNodes)
    { }
  
  ~AssignmentCollector()
  {
    util::OdbTransaction trans(*_w->getDb());
    trans([this]()
    {
      for (auto& relation : _relations)
      {
        _w->getDb()->persist(relation);
      }
    });
  }

  bool VisitBinaryOperator(clang::BinaryOperator *bop)
  {
    if( bop->isAssignmentOp() ||
      bop->isCompoundAssignmentOp() ||
      bop->isShiftAssignOp() )
    {
      makeAssignRels(bop->getLHS(), bop->getRHS());
    }

    return true;
  }

  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* ce)
  {
    auto decl = ce->getDirectCallee();
    if (!decl)
      return true;


    if (llvm::isa<clang::CXXMethodDecl>(decl))
    {
      for (int i=0; i < (int)decl->getNumParams() && i < (int)(ce->getNumArgs()) - 1; ++i)
      {
          makeAssignRels(decl->getParamDecl(i), ce->getArg(i + 1));
      }
    }
    else
    {
      for(unsigned i=0; i< ce->getNumArgs() && i < decl->getNumParams(); ++i)
      {
        makeAssignRels(decl->getParamDecl(i), ce->getArg(i));
      }
    }

    return true;
  }

  bool VisitCallExpr(clang::CallExpr *ce)
  {
    if(auto decl = ce->getDirectCallee())
    {
      for(unsigned i=0; i< ce->getNumArgs() && i < decl->getNumParams(); ++i)
      {
        makeAssignRels(decl->getParamDecl(i), ce->getArg(i));
      }
    }

    return true;
  }
  
  bool VisitVarDecl(clang::VarDecl *vd)
  {
    makeAssignRels(vd, vd->getInit());

    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *ce)
  {
    if(auto decl = ce->getConstructor())
    {
      for(unsigned i=0; i< ce->getNumArgs() && i < decl->getNumParams(); ++i)
      {
        makeAssignRels(decl->getParamDecl(i), ce->getArg(i));
      }
    }

    return true;
  }

  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *ce)
  {
    for (auto it = ce->init_begin(), e = ce->init_end(); it != e; ++it)
    {
      const clang::CXXCtorInitializer *ci = *it;
      if (!ci->getMember() || ci->getSourceOrder() == -1)
        continue;

      makeAssignRels(ci->getMember(), ci->getInit());
    }

    return true;
  }


private:
  typedef std::unordered_set<const void*> PointerSet;

  static bool isFunction(const clang::Type* type)
  {
    while (type)
    {
      if (type->isFunctionType() ||
          type->isFunctionPointerType() ||
          type->isVoidPointerType())
      {
          return true;
      }
      else if (auto parenType = type->getAs<clang::ParenType>())
      {
        type = parenType->getInnerType().getTypePtrOrNull();
      }
      else if (auto arrayType = llvm::dyn_cast<clang::ArrayType>(type))
      {
        type = arrayType->getElementType().getTypePtrOrNull();
      }
      else if (type->isPointerType() || type->isReferenceType())
      {
        type = type->getPointeeType().getTypePtrOrNull();
      }
      else
      {
        return false;
      }
    }

    return false;
  }


  template <typename T>
  PointerSet collect(T* s)
  {
    PointerSet ret;
    RefCollector rc(ret, isFunction);
    rc.collect(s);

    return ret;
  }

  template <typename T1, typename T2>
  void makeAssignRels(T1* writtens, T2* reads)
  {
    auto lhs = collect(writtens);

    auto rhs = collect(reads);

    for (auto written : lhs)
    {
      for (auto read : rhs)
      {
        makeAssignRel(written, read);
      }
    }
  }

  void makeAssignRel(const void* written, const void* read)
  {
    if (written == read)
      return;

    if (_newNodes.find(written) == _newNodes.end() &&
        _newNodes.find(read) == _newNodes.end())
    {
      return;
    }

    if (_clang2our.find(written) == _clang2our.end() ||
        _clang2our.find(read) == _clang2our.end())
    {
      return;
    }

    auto writtenNode = _clang2our.at(written);
    auto readNode    = _clang2our.at(read);

    model::CppRelation relation;
    relation.kind = model::CppRelation::Kind::Assign;
    relation.lhs  = writtenNode->mangledNameHash;
    relation.rhs  = readNode->mangledNameHash;

    _relations.emplace_back(std::move(relation));
  }

  std::shared_ptr<model::Workspace> _w;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>& _newNodes;
  std::vector<model::CppRelation> _relations;
};

} // parser
} // cc

#endif
