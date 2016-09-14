// $Id$

#ifndef CXXPARSER_NODECOLLECTOR_H
#define CXXPARSER_NODECOLLECTOR_H

#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <stack>
#include <typeinfo>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include <llvm/Support/raw_ostream.h>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>

#include <cxxparser/cxxparser.h>

#include <model/workspace.h>
#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>
#include <model/cxx/cpptypedef.h>
#include <model/cxx/cpptypedef-odb.hxx>
#include <model/cxx/cppvariable.h>
#include <model/cxx/cppvariable-odb.hxx>
#include <model/cxx/cppfunction.h>
#include <model/cxx/cppfunction-odb.hxx>
#include <model/cxx/cppfunctionpointer.h>
#include <model/cxx/cppfunctionpointer-odb.hxx>
#include <model/cxx/cppinheritance.h>
#include <model/cxx/cppinheritance-odb.hxx>
#include <model/cxx/cppfriendship.h>
#include <model/cxx/cppfriendship-odb.hxx>
#include <model/cxx/cppenum.h>
#include <model/cxx/cppenum-odb.hxx>
#include <model/cxx/cpprelation.h>
#include <model/cxx/cpprelation-odb.hxx>
#include <model/cxx/cppimplicit.h>
#include <model/cxx/cppimplicit-odb.hxx>

#include <util/streamlog.h>

#include "cxxparsesession.h"
#include "parseutil.h"

namespace cc
{
namespace parser
{

class DeclPrinter : public clang::RecursiveASTVisitor<DeclPrinter>
{
  std::ostream & _os;
  std::map<void*,std::string> _params;
  int _unk;

  std::string addParam(void* ptr)
  {
    std::ostringstream ss;
    ss << "arg_" << (_params.size() + 1);
    return _params[ptr] = ss.str();
  }

  const std::string & getParam(clang::ValueDecl* ptr)
  {
    std::string & s = _params[ptr];

    if(s.empty())
    {
      if(ptr->getNameAsString().empty())
      {
        std::ostringstream ss;
        ss << "unk_" << (++_unk);
        s = ss.str();
      }
      else
      {
        s = ptr->getNameAsString();
      }
    }

    return s;
  }
  
public:
  DeclPrinter(std::ostream & os) : _os(os), _unk(0) { }

  bool shouldVisitImplicitCode() const { return true; }
  
  bool shouldImplicitBeIgnored() const { return false; }

  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl *decl)
  {
    _os << decl->getQualifiedNameAsString() << '(';

    {
      auto it = decl->param_begin();
      if( it != decl->param_end() )
      {
        _os << removeTypeDeclarator((*it)->getType().getAsString()) << ' ' << addParam(*it);

        for(++it; it != decl->param_end(); ++it)
        {
          _os << ", " << removeTypeDeclarator((*it)->getType().getAsString()) << ' ' << addParam(*it);
        }
      }
    }
    _os << ')' << std::endl;

    {
      bool first = true;
      for(auto it=decl->init_begin(),end=decl->init_end();it!=end;++it)
      {
        clang::CXXCtorInitializer* const ini = *it;

        if(first)
        {
          _os << " : ";
          first = false;
        }
        else
        {
          _os << " , ";
        }

        if(ini->getMember())
        {
          _os << ini->getMember()->getNameAsString();
        }
        else
        if(ini->getBaseClass() && ini->getBaseClass()->getAsCXXRecordDecl())
        {
          _os << ini->getBaseClass()->getAsCXXRecordDecl()->getNameAsString();
        }
        else
        {
          _os << '?';
        }

        _os << '(';

        clang::RecursiveASTVisitor<DeclPrinter>::TraverseStmt(ini->getInit());

        _os << ')' << std::endl;
      }
    }

    {
      bool skipTraverseStmt = false;
      if (decl->getBody())
      {
        clang::CompoundStmt* cptr = llvm::dyn_cast<clang::CompoundStmt>(decl->getBody());
        if (cptr && cptr->body_empty())
          skipTraverseStmt = true;
      }
      
      if (!skipTraverseStmt)
        clang::RecursiveASTVisitor<DeclPrinter>::TraverseStmt(decl->getBody());
      else
        _os << "{}";
    }

    return true;
  }
  
  bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl *decl)
  {
    _os << decl->getQualifiedNameAsString() << "()" << std::endl;

    {
      bool skipTraverseStmt = false;
      if (decl->getBody())
      {
        clang::CompoundStmt* cptr = llvm::dyn_cast<clang::CompoundStmt>(decl->getBody());
        if (cptr && cptr->body_empty())
          skipTraverseStmt = true;
      }
      
      if (!skipTraverseStmt)
        clang::RecursiveASTVisitor<DeclPrinter>::TraverseStmt(decl->getBody());
      else
        _os << "{}";
    }

    return true;
  }

  bool TraverseCXXMethodDecl(clang::CXXMethodDecl *decl)
  {
    _os << removeTypeDeclarator(decl->getReturnType().getAsString()) << "  " << decl->getQualifiedNameAsString() << '(';

    {
      auto it = decl->param_begin();
      if( it != decl->param_end() )
      {
        _os << removeTypeDeclarator((*it)->getType().getAsString()) << ' ' << addParam(*it);

        for(++it; it != decl->param_end(); ++it)
        {
          _os << ", " << removeTypeDeclarator((*it)->getType().getAsString()) << ' ' << addParam(*it);
        }
      }
    }
    _os << ')' << std::endl;

    {
      bool skipTraverseStmt = false;
      if (decl->getBody())
      {
        clang::CompoundStmt* cptr = llvm::dyn_cast<clang::CompoundStmt>(decl->getBody());
        if (cptr && cptr->body_empty())
          skipTraverseStmt = true;
      }
      
      if (!skipTraverseStmt)
        clang::RecursiveASTVisitor<DeclPrinter>::TraverseStmt(decl->getBody());
      else
        _os << "{}";
    }

    return true;
  }

  bool TraverseCompoundStmt(clang::CompoundStmt *stmt)
  {
    _os << '{' << std::endl;
    bool res = clang::RecursiveASTVisitor<DeclPrinter>::TraverseCompoundStmt(stmt);
    _os << '}' << std::endl;
    return res;
  }

  bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr* op)
  {
    if(!op) { return true; }

    _os << "  ";

    this->TraverseStmt(op->getArg(0));

    if(clang::OO_Equal==op->getOperator())
    {
      _os << " = ";
    }
    else
    {
      _os << " (unhandled operator) ";
    }
    this->TraverseStmt(op->getArg(1));

    _os << ';' << std::endl;
    return true;
  }

  bool TraverseBinAssign(clang::BinaryOperator* op)
  {
    _os << "  ";

    this->TraverseStmt(op->getLHS());
    _os << " = ";
    this->TraverseStmt(op->getRHS());

    _os << ';' << std::endl;
    return true;
  }

  bool TraverseUnaryDeref(clang::UnaryOperator* op)
  {
    _os << '*';
    this->TraverseStmt(op->getSubExpr());
    return true;
  }

  bool TraverseCXXThisExpr(clang::CXXThisExpr*)
  {
    _os << "this";
    return true;
  }

  bool TraverseMemberExpr(clang::MemberExpr* expr)
  {
    if (expr->getValueKind() == clang::VK_XValue)
    {
      _os << "std::move( ";
    }
    
    clang::RecursiveASTVisitor<DeclPrinter>::TraverseMemberExpr(expr);

    if(expr->isArrow())
    {
      _os << "->";
    }
    else
    {
      _os << '.';
    }

    _os << expr->getMemberDecl()->getNameAsString();
    
    if (expr->getValueKind() == clang::VK_XValue)
    {
      _os << " )";
    }
    
    return true;
  }

  bool TraverseDeclRefExpr(clang::DeclRefExpr* ref)
  {
    _os << getParam(ref->getDecl());
    return true;
  }

  bool TraverseIntegerLiteral(clang::IntegerLiteral* lit)
  {
    _os << lit->getValue().toString(10, false);
    return true;
  }

  bool TraverseReturnStmt(clang::ReturnStmt* stmt)
  {
    _os << "  return ";
    clang::RecursiveASTVisitor<DeclPrinter>::TraverseReturnStmt(stmt);
    _os << ';' << std::endl;
    return true;
  }

  bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* expr)
  {
    if(expr && expr->getCallee())
    {
      _os << "  ";

      this->TraverseStmt(expr->getCallee());

      _os << '(';

      bool first = true;
      for(auto it=expr->arg_begin(),end=expr->arg_end();it!=end;++it)
      {
        clang::Expr* const arg = *it;

        if(first)
        {
          first = false;
        }
        else
        {
          _os << ", ";
        }

        this->TraverseStmt(arg);
	  }

      _os << ");" << std::endl;
    }

    return true;
  }
  
  static std::string removeTypeDeclarator(std::string str)
  {
    using namespace boost::algorithm;

    erase_all(str, "enum");
    erase_all(str, "class");
    erase_all(str, "struct");
    replace_all(str, "( ", "(");
    replace_all(str, "  ", " ");
    
    if (str.at(0) == ' ')
      str.erase(str.begin());
    
    return str;
  }
};

class NodeInfoCollector : public clang::RecursiveASTVisitor<NodeInfoCollector>
{
public:
  NodeInfoCollector(std::shared_ptr<model::Workspace> w_,
                    CxxParseSession& session_,
                    CxxAstPersister& persister_
                   ) :
    _w(w_), _clang2our(session_.clang2our),
    _newNodes(session_.newNodes),
    _persister(persister_),
    _symbolHelper(session_.astContext)
    { }

  ~NodeInfoCollector()
  {
    _persister.persistEntities(_cppTypes);
    _persister.persistEntities(_cppMembers);
    _persister.persistEntities(_cppVars);
    _persister.persistEntities(_cppFunPtrs);
    _persister.persistEntities(_cppFuns);
    _persister.persistEntities(_cppEnumConstants);
    _persister.persistEntities(_cppEnums);
    _persister.persistEntities(_cppTypedefs);
    _persister.persistEntities(_namespaces);
    _persister.persistEntities(_cppImplicits);

    util::OdbTransaction trans(*_w->getDb());
    trans([this]() {
      for (model::CppFriendship& fsh : _friendships)
      {
        _w->getDb()->persist(fsh);
      }

      for (model::CppInheritance& ih : _inheritances)
      {
        _w->getDb()->persist(ih);
      }

      for (model::CppRelation& rel : _relations)
      {
        _w->getDb()->persist(rel);
      }
    });
  }

  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }

  template <typename T>
  model::CppFunctionPtr createFunction(T* decl)
  {
    if (_newNodes.find(decl) != _newNodes.end())
    {
      auto cppFun =
        getOrCreate<model::CppFunctionPtr, model::CppFunction>(decl, _cppFuns);

      model::CppAstNodePtr node = _clang2our.at(decl);
      cppFun->astNodeId = node->id;
      cppFun->mangledNameHash = node->mangledNameHash;
      cppFun->name = getSignature(decl);
      cppFun->qualifiedName = decl->getQualifiedNameAsString();

      auto qualType = decl->getReturnType();
      cppFun->typeHash = getTypeHash(qualType);
      cppFun->qualifiedType = qualType.getAsString();

      return cppFun;
    }

    return {};
  }

  bool VisitTypedefNameDecl( clang::TypedefNameDecl *td)
  {
    if (_newNodes.find(td) == _newNodes.end())
    {
      return true;
    }

    model::CppAstNodePtr node = _clang2our.at(td);
    auto cppTd =
      getOrCreate<model::CppTypedefPtr, model::CppTypedef>(td, _cppTypedefs);

    cppTd->astNodeId = node->id;
    cppTd->mangledNameHash = node->mangledNameHash;
    cppTd->name = td->getNameAsString();
    cppTd->qualifiedName = td->getQualifiedNameAsString();

    auto qualType = td->getUnderlyingType();
    cppTd->qualifiedType = qualType.getAsString();
    cppTd->typeHash = getTypeHash(qualType);

    model::CppRelation aliasRel;
    aliasRel.kind = model::CppRelation::Kind::Alias;
    aliasRel.lhs = cppTd->typeHash;
    aliasRel.rhs = cppTd->mangledNameHash;

    _relations.emplace_back(std::move(aliasRel));

    return true;
  }

  bool TraverseFunctionDecl(clang::FunctionDecl* decl)
  {
    auto cppFun = createFunction(decl);

    if (cppFun)
    {
      _funDeclStack.push(cppFun);
    }

    bool b = clang::RecursiveASTVisitor<NodeInfoCollector>::TraverseFunctionDecl(decl);

    if (cppFun)
    {
      _funDeclStack.pop();
    }

    return b;
  }

  bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl)
  {
    // Don't do anything for implicit but unused, don't let CodeCompass show them
    if (decl->isImplicit() && !decl->isUsed(true))
      return true;
    
    auto cppFun = createFunction(decl);

    if (cppFun)
    {
      _funDeclStack.push(cppFun);

      cppFun->isVirtual = decl->isVirtual();

      auto rhsHash = getDeclHash(decl);

      for (auto it =  decl->begin_overridden_methods();
                it != decl->end_overridden_methods();
                ++it)
      {
        auto lhsHash = getDeclHash(*it);

        model::CppRelation overrideRel;

        overrideRel.kind = model::CppRelation::Kind::Override;
        overrideRel.lhs = lhsHash;
        overrideRel.rhs = rhsHash;

        _relations.emplace_back(std::move(overrideRel));
      }
    }
    
    if(decl->isImplicit() && decl->isUsed(true))
    {
      addImplicit(decl, cppFun);
    }

    bool b = clang::RecursiveASTVisitor<NodeInfoCollector>::TraverseCXXMethodDecl(decl);

    if (cppFun)
    {
      _funDeclStack.pop();
    }
    return b;
  }

  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl *decl)
  {
    // Don't do anything for implicit but unused ctors, don't let CodeCompass show them
    if (decl->isImplicit() && !decl->isUsed(true))
      return true;
    
    auto cppFun = createFunction(decl);

    if (cppFun)
    {
      _funDeclStack.push(cppFun);
      cppFun->qualifiedType = "__*tor__none__";
    }
    
    if(decl->isImplicit() && decl->isUsed(true))
    {
      addImplicit(decl, cppFun);
    }

    bool b = clang::RecursiveASTVisitor<NodeInfoCollector>::TraverseCXXConstructorDecl(decl);

    if (cppFun)
    {
      _funDeclStack.pop();
    }

    return b;
  }

  bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl *decl)
  {
    // Don't do anything for implicit but unused dtors, don't let CodeCompass show them
    if (decl->isImplicit() && !decl->isUsed(true))
      return true;
    
    auto cppFun = createFunction(decl);

    if (cppFun)
    {
      _funDeclStack.push(cppFun);
      cppFun->qualifiedType = "__*tor__none__";
    }

    if(decl->isImplicit() && decl->isUsed(true))
    {
      addImplicit(decl, cppFun);
    }

    bool b = clang::RecursiveASTVisitor<NodeInfoCollector>::TraverseCXXDestructorDecl(decl);

    if (cppFun)
    {
      _funDeclStack.pop();
    }

    return b;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl *nd)
  {
    if (_newNodes.find(nd) == _newNodes.end())
    {
      return true;
    }

    model::CppNamespacePtr ns = std::make_shared<model::CppNamespace>();
    auto astNode = _clang2our.at(nd);
    ns->name = nd->getNameAsString();
    ns->qualifiedName = astNode->mangledName;
    ns->mangledNameHash = astNode->mangledNameHash;

    _namespaces.push_back(ns);

    return true;
  }

  bool VisitRecordDecl(const clang::RecordDecl *rd)
  {
    if (!rd->isCompleteDefinition())
      return true;

    if (_newNodes.find(rd) == _newNodes.end())
    {
      return true;
    }

    model::CppAstNodePtr node = _clang2our.at(rd);
    model::CppTypePtr cppType =
      getOrCreate<model::CppTypePtr, model::CppType>(rd, _cppTypes);
    cppType->astNodeId = node->id;
    cppType->mangledNameHash = node->mangledNameHash;
    cppType->name = _symbolHelper.getTypeName(rd);
    cppType->qualifiedName = _symbolHelper.getTypeQualifiedName(rd);

    for( auto it = rd->field_begin(); it != rd->field_end(); ++it)
    {
      addVarInfo(*it);

      if (_newNodes.find(*it) != _newNodes.end())
      {
        model::CppMemberTypePtr field =
          getOrCreate<model::CppMemberTypePtr, model::CppMemberType>(*it, _cppMembers);
        
        field->typeHash = cppType->mangledNameHash;
        field->memberAstNode = _clang2our.at(*it);
        
        auto memberType = it->getType();
        auto memberTypeHash = getTypeHash(memberType);
        field->memberTypeHash = memberTypeHash;

        field->kind = model::CppMemberType::Kind::Field;
        field->visibility = getMemberVisibility(*it);
      }
    }

    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl *ec)
  {
    if (_newNodes.find(ec) != _newNodes.end())
    {
      model::CppAstNodePtr node = _clang2our.at(ec);
      model::CppEnumConstantPtr cppEnumC =
        getOrCreate<model::CppEnumConstantPtr, model::CppEnumConstant>(ec, _cppEnumConstants);
      cppEnumC->astNodeId = node->id;
      cppEnumC->mangledNameHash = node->mangledNameHash;
      cppEnumC->name = ec->getNameAsString();
      cppEnumC->qualifiedName = ec->getQualifiedNameAsString();

      cppEnumC->value = ec->getInitVal().getLimitedValue();

      if (_activeEnum)
      {
        _activeEnum->enumConstants.push_back(cppEnumC);
      }
    }

    return true;
  }

  bool TraverseEnumDecl(clang::EnumDecl *ed)
  {
    bool b = true;

    if (!ed->isCompleteDefinition())
      return b;

    if (_newNodes.find(ed) != _newNodes.end())
    {
      std::string enumName = ed->getNameAsString();
      if (enumName.empty())
      {
        enumName = "<anonymous>";
      }

      model::CppAstNodePtr node = _clang2our.at(ed);
      model::CppEnumPtr cppEnum =
        getOrCreate<model::CppEnumPtr, model::CppEnum>(ed, _cppEnums);
      cppEnum->astNodeId = node->id;
      cppEnum->mangledNameHash = node->mangledNameHash;
      cppEnum->name = enumName;
      cppEnum->qualifiedName = ed->getQualifiedNameAsString();

      _activeEnum = cppEnum;
      b = clang::RecursiveASTVisitor<NodeInfoCollector>::TraverseEnumDecl(ed);
      _activeEnum.reset();
    }

    return b;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *rd)
  {
    if (!rd->isCompleteDefinition())
      return true;

    if (_newNodes.find(rd) == _newNodes.end())
    {
      // TODO: warn
      return true;
    }

    model::CppTypePtr cppType = getOrCreate<model::CppTypePtr, model::CppType>(
      rd, _cppTypes);

    cppType->isAbstract = rd->isAbstract();
    cppType->isPOD = rd->isPOD();
    for( auto it = rd->method_begin(); it != rd->method_end(); ++it)
    {
      if (_newNodes.find(*it) != _newNodes.end())
      {
        model::CppMemberTypePtr method =
          getOrCreate<model::CppMemberTypePtr, model::CppMemberType>(*it, _cppMembers);

        method->typeHash = cppType->mangledNameHash;
        method->memberAstNode = _clang2our.at(*it);

        auto memberType = it->getType();
        method->memberTypeHash = getTypeHash(memberType);

        method->isStatic = (*it)->isStatic();
        method->kind = model::CppMemberType::Kind::Method;
        method->visibility = getMemberVisibility(*it);
      }
    }

    // Inheritance
    for (auto baseIt = rd->bases_begin(); baseIt != rd->bases_end(); ++baseIt)
    {
      const clang::Type* baseType = baseIt->getType().getTypePtr();
      const clang::CXXRecordDecl* baseDecl =
        baseType ? baseType->getAsCXXRecordDecl() : nullptr;

      if (!baseDecl &&
           baseType &&
           baseType->getTypeClass() == clang::Type::TemplateSpecialization)
      {
        // Minimal support for a template base class
        const clang::TemplateDecl* templ =
          baseType->getAs<clang::TemplateSpecializationType>()->
            getTemplateName().getAsTemplateDecl();
        if (auto classTempl = llvm::dyn_cast<clang::ClassTemplateDecl>(templ))
        {
          baseDecl = classTempl->getTemplatedDecl();
        }
      }

      if (baseDecl)
      {
        model::CppInheritance inheritance;
        inheritance.derived = cppType->mangledNameHash;
        inheritance.base = util::fnvHash(
          _symbolHelper.getTypeQualifiedName(baseDecl));
        inheritance.isVirtual = baseIt->isVirtual();
        inheritance.visibility = clangAccessToVisibility(
          baseIt->getAccessSpecifier());

        _inheritances.emplace_back(std::move(inheritance));
      }
      else
      {
        SLog(util::DEBUG)
          << "No CXXRecordDecl for " << baseIt->getType().getAsString()
          << " in definition of " << rd->getQualifiedNameAsString()
          << " (type is " << (baseType ? "not null" : "null")
          << ", decl is " << (baseDecl ? "not null" : "null") << ")";
      }
    }

    // Friends
    for (auto friendIt = rd->friend_begin(); friendIt != rd->friend_end();
      ++friendIt)
    {
      // I know it's ugly as hell but I couldn't come up with a nice solution
      // to this problem
      if ((*friendIt)->getFriendType()
        &&
        (*friendIt)->getFriendType()->getType().getTypePtr()
        &&
        (*friendIt)->getFriendType()->getType().getTypePtr()->getAsCXXRecordDecl())
      {
        auto friendDecl =
          (*friendIt)->getFriendType()->getType().getTypePtr()->getAsCXXRecordDecl();

        auto friendHash = util::fnvHash(
          _symbolHelper.getTypeQualifiedName(friendDecl));

        model::CppFriendship friendship;
        friendship.target = cppType->mangledNameHash;
        friendship.theFriend = friendHash;
        _friendships.emplace_back(std::move(friendship));
      }
      else
      {
        SLog(util::DEBUG)
          << "No CXXRecordDecl for a friend declaration in "
          << rd->getQualifiedNameAsString();
      }
    }

    return true;
  }

  bool VisitVarDecl( clang::VarDecl* decl )
  {

    if (_newNodes.find(decl) == _newNodes.end())
    {
        return true;
      }

    addVarInfo(decl);
    //If the stack is empty, we are visiting a global variable
    if( !_funDeclStack.empty())
    {
      if (auto varPtr = getVariablePtr(decl))
      {
        if( llvm::isa<clang::ParmVarDecl>(decl) )
          _funDeclStack.top()->parameters.push_back(varPtr);
        else
          _funDeclStack.top()->locals.push_back(varPtr);
      }
    }
    // Static data members are members of the containing class (in CodeCompass)
    else if (decl->isStaticDataMember() && decl->isFirstDecl())
    {
      clang::RecordDecl* record =
          llvm::cast<clang::RecordDecl>(decl->getDeclContext());

      if (record && (_newNodes.find(record) != _newNodes.end()))
      {
        model::CppTypePtr cppType =
            getOrCreate<model::CppTypePtr, model::CppType>(record, _cppTypes);

        model::CppMemberTypePtr field =
          getOrCreate<model::CppMemberTypePtr, model::CppMemberType>(
            decl, _cppMembers);
        
        field->typeHash = cppType->mangledNameHash;
        field->memberAstNode = _clang2our.at(decl);
        
        auto memberType = decl->getType();
        auto memberTypeHash = getTypeHash(memberType);
        field->memberTypeHash = getTypeHash(memberType);

        field->isStatic = true;
        field->kind = model::CppMemberType::Kind::Field;
        field->visibility = getMemberVisibility(decl);
      }
    }

    return true;
  }

  model::CppVariablePtr getVariablePtr(clang::DeclaratorDecl* decl)
  {
    if (_cppVars.find(decl) != _cppVars.end())
    {
      return _cppVars[decl];
    }

    if (_cppFunPtrs.find(decl) != _cppFunPtrs.end())
    {
      return _cppFunPtrs[decl];
    }

    return {};
  }

private:

  std::string getSignature(clang::FunctionDecl* decl)
  {
    std::string s = "";
    s += decl->getReturnType().getAsString() + " " + decl->getNameAsString() + "(";
    auto it = decl->param_begin();
    if( it != decl->param_end() )
    {
      s += (*it)->getType().getAsString();

      for( ++it; it != decl->param_end(); ++it)
      {
        s += ", " + (*it)->getType().getAsString();
      }
    }
    s += ")";

    return s;
  }

  template<typename AstInfoPtr, typename AstInfo>
  AstInfoPtr getOrCreate(const void* c_, std::map< const void*, AstInfoPtr>& storage_)
  {
    auto it = storage_.find(c_);
    if(it != storage_.end())
      return it->second;

    AstInfoPtr t( new AstInfo );
    storage_.insert(std::make_pair(c_, t));
    return t;
  }

  void addVarInfo(clang::ValueDecl* t)
  {
    if (_newNodes.find(t) == _newNodes.end())
    {
      return;
    }

    model::CppVariablePtr var;
    if (isFunctionPointer(t))
    {
      var =
        getOrCreate<model::CppFunctionPointerPtr, model::CppFunctionPointer>(
          t, _cppFunPtrs);
    }
    else
    {
      var =
        getOrCreate<model::CppVariablePtr, model::CppVariable>(t, _cppVars);
    }

    addVarInfo(t, var);
  }

  void addVarInfo(clang::ValueDecl* v, model::CppVariablePtr var)
  {
    model::CppAstNodePtr node = _clang2our.at(v);
    var->astNodeId = node->id;
    var->mangledNameHash = node->mangledNameHash;
    var->name = v->getNameAsString();
    var->qualifiedName = v->getQualifiedNameAsString();

    auto qualType = v->getType();
    var->typeHash = getTypeHash(qualType);
    var->qualifiedType = qualType.getAsString();
  }

  void addImplicit(clang::CXXMethodDecl* decl, model::CppFunctionPtr _funcPtr)
  {
    try
    {
      clang::CXXRecordDecl* rd = decl->getParent();

      model::CppAstNodePtr declNode = _clang2our.at(decl);
      model::CppAstNodePtr rdNode = _clang2our.at(rd);

      std::ostringstream ss;
      DeclPrinter dp(ss);
      dp.TraverseDecl(static_cast<clang::Decl*>(decl));
      
      if (!dp.shouldImplicitBeIgnored())
      {
        model::CppImplicitPtr cppImplicit =
          getOrCreate<model::CppImplicitPtr, model::CppImplicit>(decl->getCanonicalDecl(), _cppImplicits);
        
        cppImplicit->name = DeclPrinter::removeTypeDeclarator(getSignature(decl));
        
        cppImplicit->qualifiedName = decl->getQualifiedNameAsString();
        cppImplicit->mangledNameHash = declNode->mangledNameHash;
        cppImplicit->typeHash = rdNode->mangledNameHash;
        cppImplicit->code = ss.str();
        
        if (_funcPtr)
        {
          _funcPtr->name = DeclPrinter::removeTypeDeclarator(_funcPtr->name);
          _funcPtr->qualifiedType = DeclPrinter::removeTypeDeclarator(_funcPtr->qualifiedType);
        }
      } else {
        SLog(util::DEBUG) << "Implicit method was found, but the visitor decided that it should be ignored:\n\n" << ss.str();
      }
    }
    catch(std::out_of_range & ex)
    {
      // absorb it
    }
  }

  std::string getTypeMangledName(clang::QualType qualType)
  {
    auto type = getStrippedType(qualType);

    if (!type)
    {
      SLog() << "TypePtr became NULL!";
      return "";
    }

    if (auto td = type->getAs<clang::TypedefType>())
    {
      if (auto tDecl = td->getDecl())
      {
        if (_newNodes.find(tDecl) != _newNodes.end())
        {
          return _symbolHelper.getMangledNameOfTypedef(tDecl,
            _clang2our.at(tDecl));
        }
      }
    }
    else if (auto record = type->getAsCXXRecordDecl())
    {
      return _symbolHelper.getMangledNameOfType(record, {});
    }
    // Enums
    else if (auto enumType = type->getAs<clang::EnumType>())
    {
      if (auto decl = enumType->getDecl())
      {
        return _symbolHelper.getMangledNameOfEnum(decl, {});
      }
    }

    //SLog() << "Couldn't find entity for: " << type->getTypeClassName();

    return "";
  }

  model::HashType getTypeHash(clang::QualType qualType)
  {
    auto mn = getTypeMangledName(qualType);

    if (mn.empty())
    {
      return 0;
    }

    return util::fnvHash(mn);
  }

  template <typename DECL>
  unsigned long long getDeclHash(const DECL* d)
  {
    clang::GlobalDecl gDecl(d);
    auto mangledName = _symbolHelper.getMangledName(gDecl);
    return util::fnvHash(mangledName);
  }

  model::Visibility getMemberVisibility(const clang::Decl* decl_) const
  {
    return clangAccessToVisibility(decl_->getAccess());
  }

  model::Visibility clangAccessToVisibility(clang::AccessSpecifier access) const
  {
    switch(access)
    {
      case clang::AS_public:
        return model::Public;
        break;

      case clang::AS_protected:
        return model::Protected;
        break;

      case clang::AS_private:
        return model::Private;
        break;

      case clang::AS_none:
        return model::Public;
        break;
    }

    // Hush now sweet CDT...
    return model::Public;
  }

  std::shared_ptr<model::Workspace> _w;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>& _newNodes;
  CxxAstPersister& _persister;

  std::map<const void*, model::CppTypePtr> _cppTypes;
  std::map<const void*, model::CppMemberTypePtr> _cppMembers;
  std::map<const void*, model::CppVariablePtr> _cppVars;
  std::map<const void*, model::CppFunctionPtr> _cppFuns;
  std::map<const void*, model::CppFunctionPointerPtr> _cppFunPtrs;
  std::map<const void*, model::CppEnumPtr> _cppEnums;
  std::map<const void*, model::CppEnumConstantPtr> _cppEnumConstants;
  std::map<const void*, model::CppTypedefPtr> _cppTypedefs;
  std::map<const void*, model::CppImplicitPtr> _cppImplicits;
  std::vector<model::CppNamespacePtr> _namespaces;

  std::vector<model::CppInheritance> _inheritances;
  std::vector<model::CppFriendship> _friendships;
  std::vector<model::CppRelation> _relations;

  std::stack<model::CppFunctionPtr> _funDeclStack;
  model::CppEnumPtr _activeEnum;

  SymbolHelper _symbolHelper;
};

} // parser
} // cc

#endif
