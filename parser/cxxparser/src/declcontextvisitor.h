#ifndef CXXPARSER_DECLCONTEXTVISITOR_H
#define CXXPARSER_DECLCONTEXTVISITOR_H

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

namespace cc
{
namespace parser
{

class DeclContextVisitor : public clang::RecursiveASTVisitor<DeclContextVisitor>
{
public:
  DeclContextVisitor(std::shared_ptr<model::Workspace> w_,
                    CxxParseSession& session_,
                    CxxAstPersister& persister_
                   ) :
    _w(w_), _clang2our(session_.clang2our),
    _newNodes(session_.newNodes), _persister(persister_)
  {
    globalNsHash = _persister.getGlobalNamespace().mangledNameHash;
  }
  
  ~DeclContextVisitor()
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

  bool TraverseNamespaceDecl(clang::NamespaceDecl *nd)
  {
    pushContext(nd);

    bool b = clang::RecursiveASTVisitor<DeclContextVisitor>::TraverseNamespaceDecl(nd);

    popContext();

    return b;
  }

  bool TraverseCXXRecordDecl(clang::CXXRecordDecl *rd)
  {
    pushContext(rd);

    bool b = clang::RecursiveASTVisitor<DeclContextVisitor>::TraverseCXXRecordDecl(rd);

    popContext();

    return b;
  }

  bool TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* rd)
  {
    pushContext(rd);

    bool b = clang::RecursiveASTVisitor<DeclContextVisitor>::TraverseClassTemplatePartialSpecializationDecl(rd);

    popContext();

    return b;
  }

  bool TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* rd)
  {
    pushContext(rd);

    bool b = clang::RecursiveASTVisitor<DeclContextVisitor>::TraverseClassTemplateSpecializationDecl(rd);

    popContext();

    return b;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl *nd)
  {
    auto thisHash = popContext();

    addToContext(nd);

    declContext.push(thisHash);

    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *rd)
  {
    auto thisHash = popContext();

    addToContext(rd);

    declContext.push(thisHash);

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *vd)
  {
    if (vd->isLocalVarDecl() || llvm::isa<clang::ParmVarDecl>(vd))
    {
      return true;
    }

    addToContext(vd);

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *fd)
  {
    addToContext(fd);

    return true;
  }

private:
  void pushContext(clang::NamedDecl *decl)
  {
    declContext.push(getHash(decl));
  }

  model::HashType popContext()
  {
    auto ret = declContext.top();

    declContext.pop();

    return ret;
  }

  bool inNewNodes(clang::NamedDecl *decl)
  {
    if (_newNodes.find(decl) != _newNodes.end())
    {
      return true;
    }

    return false;
  }

  void addToContext(clang::NamedDecl *d)
  {
    if (!inNewNodes(d))
      return;

    model::CppRelation rel;

    rel.kind = model::CppRelation::Kind::DeclContext;

    if (declContext.empty())
    {
      rel.lhs = globalNsHash;
    }
    else
    {
      rel.lhs = declContext.top();
    }

    rel.rhs = getHash(d);
    _relations.emplace_back(std::move(rel));
  }

  model::HashType getHash(clang::NamedDecl *d)
  {
    return _clang2our.at(d)->mangledNameHash;
  }

  std::shared_ptr<model::Workspace> _w;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>& _newNodes;
  CxxAstPersister& _persister;

  std::stack<model::HashType> declContext;
  model::HashType globalNsHash;

  std::vector<model::CppRelation> _relations;
};

} // parser
} // cc

#endif // CXXPARSER_DECLCONTEXTVISITOR_H
