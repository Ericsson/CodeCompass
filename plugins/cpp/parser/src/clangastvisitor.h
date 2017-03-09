#ifndef CC_PARSER_CLANGASTVISITOR_H
#define CC_PARSER_CLANGASTVISITOR_H

#include <memory>
#include <mutex>
#include <type_traits>
#include <stack>

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <llvm/Support/raw_ostream.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppenum.h>
#include <model/cppenum-odb.hxx>
#include <model/cppfriendship.h>
#include <model/cppfriendship-odb.hxx>
#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/cppinheritance.h>
#include <model/cppinheritance-odb.hxx>
#include <model/cppnamespace.h>
#include <model/cppnamespace-odb.hxx>
#include <model/cpptype.h>
#include <model/cpptype-odb.hxx>
#include <model/cpptypedef.h>
#include <model/cpptypedef-odb.hxx>

#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>
#include <util/hash.h>
#include <util/odbtransaction.h>
#include <util/logutil.h>

#include "manglednamecache.h"
#include "filelocutil.h"
#include "symbolhelper.h"

namespace cc
{
namespace parser
{

/**
 * A class to fill C++ related database tables.
 *
 * This class visits all AST nodes and stores information of the important ones.
 * The class handles only one translation unit (TRU). The nodes of the syntax
 * tree are stored in memory while processing the TRU, and then they are
 * persisted in the destructor.
 *
 * Some information may be gathered from several visitor functions. E.g. for
 * building a model::CppFunction object we have to collect the function's
 * parameters and local variables. clang::FunctionDecl and clang::VarDecl are
 * quite far from each other. The connection between them is established by a
 * stack which stores the currently iterated function at the top. This way the
 * parameters and local variables of the function find their parent at the top
 * of the stack. If stack is used then they are pushed and popped in the
 * corresponding Traverse... function.
 */
class ClangASTVisitor : public clang::RecursiveASTVisitor<ClangASTVisitor>
{
public:
  ClangASTVisitor(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
    : _isImplicit(false),
      _ctx(ctx_),
      _clangSrcMgr(astContext_.getSourceManager()),
      _fileLocUtil(astContext_.getSourceManager()),
      _mngCtx(astContext_.createMangleContext()),
      _cppSourceType("CPP"),
      _mangledNameCache(mangledNameCache_),
      _clangToAstNodeId(clangToAstNodeId_)
  {
  }

  ~ClangASTVisitor()
  {
    _ctx.srcMgr.persistFiles();

    (util::OdbTransaction(_ctx.db))([this]{
      persistAll(_astNodes);
      persistAll(_enumConstants);
      persistAll(_enums);
      persistAll(_types);
      persistAll(_typedefs);
      persistAll(_variables);
      persistAll(_namespaces);
      persistAll(_members);
      persistAll(_inheritances);
      persistAll(_friends);
      persistAll(_functions);
      persistAll(_relations);
    });
  }

  bool shouldVisitImplicitCode() const { return true; }
  bool shouldVisitTemplateInstantiations() const { return true; }

  bool TraverseDecl(clang::Decl* decl_)
  {
    bool prevIsImplicit = _isImplicit;

    if (decl_)
      _isImplicit = decl_->isImplicit() || _isImplicit;

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseDecl(decl_);

    _isImplicit = prevIsImplicit;

    return b;
  }

  bool TraverseFunctionDecl(clang::FunctionDecl* fd_)
  {
    _functionStack.push(std::make_shared<model::CppFunction>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseFunctionDecl(fd_);

    if (_functionStack.top()->astNodeId)
      _functions.push_back(_functionStack.top());
    _functionStack.pop();

    return b;
  }

  bool TraverseCXXMethodDecl(clang::CXXMethodDecl* fd_)
  {
    _functionStack.push(std::make_shared<model::CppFunction>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseCXXMethodDecl(fd_);

    if (_functionStack.top()->astNodeId)
      _functions.push_back(_functionStack.top());
    _functionStack.pop();

    return b;
  }

  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* fd_)
  {
    _functionStack.push(std::make_shared<model::CppFunction>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseCXXConstructorDecl(fd_);

    if (_functionStack.top()->astNodeId)
      _functions.push_back(_functionStack.top());
    _functionStack.pop();

    return b;
  }

  bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl* fd_)
  {
    _functionStack.push(std::make_shared<model::CppFunction>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseCXXDestructorDecl(fd_);

    if (_functionStack.top()->astNodeId)
      _functions.push_back(_functionStack.top());
    _functionStack.pop();

    return b;
  }

  bool TraverseCXXConversionDecl(clang::CXXConversionDecl* fd_)
  {
    _functionStack.push(std::make_shared<model::CppFunction>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseCXXConversionDecl(fd_);

    if (_functionStack.top()->astNodeId)
      _functions.push_back(_functionStack.top());
    _functionStack.pop();

    return b;
  }

  bool TraverseRecordDecl(clang::RecordDecl* rd_)
  {
    _typeStack.push(std::make_shared<model::CppType>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseRecordDecl(rd_);

    if (_typeStack.top()->astNodeId)
      _types.push_back(_typeStack.top());
    _typeStack.pop();

    return b;
  }

  bool TraverseCXXRecordDecl(clang::CXXRecordDecl* rd_)
  {
    _typeStack.push(std::make_shared<model::CppType>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseCXXRecordDecl(rd_);

    if (_typeStack.top()->astNodeId)
      _types.push_back(_typeStack.top());
    _typeStack.pop();

    return b;
  }

  bool TraverseClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl* rd_)
  {
    _typeStack.push(std::make_shared<model::CppType>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseClassTemplateSpecializationDecl(rd_);

    if (_typeStack.top()->astNodeId)
      _types.push_back(_typeStack.top());
    _typeStack.pop();

    return b;
  }

  bool TraverseClassTemplatePartialSpecializationDecl(
    clang::ClassTemplatePartialSpecializationDecl* rd_)
  {
    _typeStack.push(std::make_shared<model::CppType>());

    bool b = clang::RecursiveASTVisitor<
      ClangASTVisitor>::TraverseClassTemplatePartialSpecializationDecl(rd_);

    if (_typeStack.top()->astNodeId)
      _types.push_back(_typeStack.top());
    _typeStack.pop();

    return b;
  }

  bool TraverseEnumDecl(clang::EnumDecl* ed_)
  {
    _enumStack.push(std::make_shared<model::CppEnum>());

    bool b = clang::RecursiveASTVisitor<ClangASTVisitor>::TraverseEnumDecl(ed_);

    if (_enumStack.top()->astNodeId)
      _enums.push_back(_enumStack.top());
    _enumStack.pop();

    return b;
  }

  bool VisitTypeLoc(clang::TypeLoc tl_)
  {
    clang::QualType t = tl_.getType();
    const clang::Type* type = t.getTypePtrOrNull();
    if (!type) return true;

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();
    astNode->location = getFileLoc(tl_.getLocStart(), tl_.getLocEnd());
    astNode->astType = model::CppAstNode::AstType::TypeLocation;
    astNode->symbolType = model::CppAstNode::SymbolType::Type;

    if (const clang::TypedefType* tdType = type->getAs<clang::TypedefType>())
    {
      if (const clang::TypedefNameDecl* td = tdType->getDecl())
      {
        astNode->astValue = td->getNameAsString();
        astNode->mangledName = getMangledName(_mngCtx, td);
        astNode->symbolType = model::CppAstNode::SymbolType::Typedef;
      }
    }
    else if (const clang::CXXRecordDecl* rd = type->getAsCXXRecordDecl())
    {
      astNode->astValue = rd->getNameAsString();
      astNode->mangledName = getMangledName(_mngCtx, rd);
    }
    else if (const clang::EnumType* enumType = type->getAs<clang::EnumType>())
    {
      if (const clang::EnumDecl* ed = enumType->getDecl())
      {
        astNode->astValue = ed->getNameAsString();
        astNode->mangledName = getMangledName(_mngCtx, ed);
        astNode->symbolType = model::CppAstNode::SymbolType::Enum;
      }
    }
    else
    {
      return true;
    }

    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(0, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* rd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = rd_->getNameAsString();
    astNode->location = getFileLoc(rd_->getLocStart(), rd_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, rd_, astNode->location);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Type;
    astNode->astType
      = rd_->isCompleteDefinition()
      ? model::CppAstNode::AstType::Definition
      : model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(rd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppType ---//

    if (!rd_->isThisDeclarationADefinition())
      return true;

    model::CppTypePtr cppType = _typeStack.top();

    cppType->astNodeId = astNode->id;
    cppType->mangledNameHash = astNode->mangledNameHash;
    cppType->name = rd_->getNameAsString();
    cppType->qualifiedName = rd_->getQualifiedNameAsString();
    if (const clang::CXXRecordDecl* crd
        = llvm::dyn_cast<clang::CXXRecordDecl>(rd_))
    {
      cppType->isAbstract = crd->isAbstract();
      cppType->isPOD = crd->isPOD();
    }

    if (clang::CXXRecordDecl* crd = llvm::dyn_cast<clang::CXXRecordDecl>(rd_))
    {
      //--- CppInheritance ---//

      for (auto it = crd->bases_begin(); it != crd->bases_end(); ++it)
      {
        const clang::Type* baseType = it->getType().getTypePtr();
        const clang::CXXRecordDecl* baseDecl
          = baseType
          ? baseType->getAsCXXRecordDecl()
          : nullptr;

        if (baseDecl)
        {
          model::CppInheritancePtr inheritance
            = std::make_shared<model::CppInheritance>();
          _inheritances.push_back(inheritance);

          inheritance->derived = cppType->mangledNameHash;
          inheritance->base
            = util::fnvHash(getMangledName(_mngCtx, baseDecl));
          inheritance->isVirtual = it->isVirtual();
          inheritance->visibility = getVisibility(it->getAccessSpecifier());

          //--- AST node for inherited class ---//

          model::CppAstNodePtr baseNode = std::make_shared<model::CppAstNode>();

          baseNode->astValue = baseDecl->getNameAsString();
          baseNode->location = getFileLoc(it->getLocStart(), it->getLocEnd());
          baseNode->mangledName
            = getMangledName(_mngCtx, baseDecl, baseNode->location);
          baseNode->mangledNameHash = util::fnvHash(baseNode->mangledName);
          baseNode->symbolType = model::CppAstNode::SymbolType::Type;
          baseNode->astType = model::CppAstNode::AstType::Usage;
          baseNode->id = model::createIdentifier(*baseNode);

          if (insertToCache(0, baseNode))
            _astNodes.push_back(baseNode);
        }
      }

      //--- CppFriendship ---//

      for (auto it = crd->friend_begin(); it != crd->friend_end(); ++it)
      {
        const clang::Type* type = nullptr;
        clang::CXXRecordDecl* cxxRecordDecl = nullptr;
        clang::TypeSourceInfo* tsi = (*it)->getFriendType();

        if (tsi) type = tsi->getType().getTypePtr();
        if (type) cxxRecordDecl = type->getAsCXXRecordDecl();

        if (cxxRecordDecl)
        {
          //--- Friend classes ---//

          model::CppFriendshipPtr friendship
            = std::make_shared<model::CppFriendship>();
          _friends.push_back(friendship);

          friendship->target = cppType->mangledNameHash;
          friendship->theFriend
            = util::fnvHash(getMangledName(_mngCtx, cxxRecordDecl));
        }
        else if (clang::NamedDecl* friendDecl = (*it)->getFriendDecl())
        {
          //--- Friend functions ---//

          model::CppFriendshipPtr friendship
            = std::make_shared<model::CppFriendship>();
          _friends.push_back(friendship);

          friendship->target = cppType->mangledNameHash;
          friendship->theFriend
            = util::fnvHash(getMangledName(_mngCtx, friendDecl));
        }
      }
    }

    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* ed_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = ed_->getNameAsString();
    astNode->location = getFileLoc(ed_->getLocStart(), ed_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, ed_);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Enum;
    astNode->astType
      = ed_->isComplete()
      ? model::CppAstNode::AstType::Definition
      : model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ed_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppEnum ---//

    model::CppEnumPtr cppEnum = _enumStack.top();

    std::string name = ed_->getNameAsString();
    if (name.empty())
      name = "<anonymous>";

    cppEnum->astNodeId = astNode->id;
    cppEnum->mangledNameHash = astNode->mangledNameHash;
    cppEnum->name = name;
    cppEnum->qualifiedName = ed_->getQualifiedNameAsString();

    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl* ec_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = ec_->getNameAsString();
    astNode->location = getFileLoc(ec_->getLocStart(), ec_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, ec_);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::EnumConstant;
    astNode->astType = model::CppAstNode::AstType::Definition;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ec_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppEnumConstant ---//

    model::CppEnumConstantPtr enumConstant
      = std::make_shared<model::CppEnumConstant>();
    _enumConstants.push_back(enumConstant);

    enumConstant->astNodeId = astNode->id;
    enumConstant->mangledNameHash = astNode->mangledNameHash;
    enumConstant->name = ec_->getNameAsString();
    enumConstant->qualifiedName = ec_->getQualifiedNameAsString();
    enumConstant->value = ec_->getInitVal().getLimitedValue();

    _enumStack.top()->enumConstants.push_back(enumConstant);

    return true;
  }

  bool VisitTypedefNameDecl(clang::TypedefNameDecl* td_)
  {
    //--- CppAstNode ---//

    // TODO: Originally mangled name was appended by some suffix. Why?
    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = td_->getNameAsString();
    astNode->location = getFileLoc(td_->getLocStart(), td_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, td_);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Typedef;
    astNode->astType = model::CppAstNode::AstType::Definition;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(td_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppTypedef ---//

    model::CppTypedefPtr cppTypedef = std::make_shared<model::CppTypedef>();
    _typedefs.push_back(cppTypedef);

    clang::QualType qualType = td_->getUnderlyingType();

    cppTypedef->astNodeId = astNode->id;
    cppTypedef->mangledNameHash = astNode->mangledNameHash;
    cppTypedef->name = td_->getNameAsString();
    cppTypedef->qualifiedName = td_->getQualifiedNameAsString();
    cppTypedef->typeHash = util::fnvHash(getMangledName(_mngCtx, qualType));
    cppTypedef->qualifiedType = qualType.getAsString();

    //--- CppAstNode for aliased type ---//

    const clang::Type* type = getStrippedType(qualType);
    if (const clang::CXXRecordDecl* rd = type->getAsCXXRecordDecl())
    {
      clang::TypeLoc loc = td_->getTypeSourceInfo()->getTypeLoc();

      model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

      astNode->astValue = rd->getNameAsString();
      astNode->location = getFileLoc(loc.getLocStart(), loc.getLocEnd());
      astNode->mangledName = getMangledName(_mngCtx, rd);
      astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
      astNode->symbolType = model::CppAstNode::SymbolType::Type;
      astNode->astType = model::CppAstNode::AstType::Usage;

      astNode->id = model::createIdentifier(*astNode);

      if (insertToCache(0, astNode))
        _astNodes.push_back(astNode);
    }

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* fn_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = fn_->getNameAsString();
    astNode->location = getFileLoc(fn_->getLocStart(), fn_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, fn_);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType
      = fn_->isThisDeclarationADefinition()
      ? model::CppAstNode::AstType::Definition
      : model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(fn_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppFunction ---//

    model::CppFunctionPtr cppFunction = _functionStack.top();

    clang::QualType qualType = fn_->getReturnType();

    cppFunction->astNodeId = astNode->id;
    cppFunction->mangledNameHash = astNode->mangledNameHash;
    cppFunction->name = getSignature(fn_);
    cppFunction->qualifiedName = fn_->getQualifiedNameAsString();
    cppFunction->typeHash = util::fnvHash(getMangledName(_mngCtx, qualType));
    cppFunction->qualifiedType = qualType.getAsString();

    clang::CXXMethodDecl* md = llvm::dyn_cast<clang::CXXMethodDecl>(fn_);

    //--- Tags ---//

    if (md)
    {
      if (md->isVirtual())
        cppFunction->tags.insert(model::Tag::Virtual);

      if (md->isStatic())
        cppFunction->tags.insert(model::Tag::Static);

      if (llvm::isa<clang::CXXConstructorDecl>(md))
        cppFunction->tags.insert(model::Tag::Constructor);

      if (llvm::isa<clang::CXXDestructorDecl>(md))
        cppFunction->tags.insert(model::Tag::Destructor);
    }

    if (_isImplicit)
      cppFunction->tags.insert(model::Tag::Implicit);

    //--- CppAstNode for the return type ---//

    model::CppAstNodePtr typeAstNode = std::make_shared<model::CppAstNode>();

    const clang::Type* type = getStrippedType(qualType);
    if (const clang::CXXRecordDecl* rd = type->getAsCXXRecordDecl())
    {
      typeAstNode->astValue = rd->getNameAsString();
      typeAstNode->location = getFileLoc(
        fn_->getReturnTypeSourceRange().getBegin(),
        fn_->getReturnTypeSourceRange().getEnd());
      typeAstNode->mangledName = getMangledName(_mngCtx, rd);
      typeAstNode->mangledNameHash = util::fnvHash(typeAstNode->mangledName);
      typeAstNode->symbolType = model::CppAstNode::SymbolType::Type;
      typeAstNode->astType = model::CppAstNode::AstType::ReturnTypeLoc;

      typeAstNode->id = model::createIdentifier(*typeAstNode);

      if (insertToCache(0, typeAstNode))
        _astNodes.push_back(typeAstNode);
    }

    //--- CppMemberType ---//

    // If empty then this is a global declaration.
    if (md && !_typeStack.empty())
    {
      model::CppMemberTypePtr member = std::make_shared<model::CppMemberType>();
      _members.push_back(member);

      member->memberAstNode = astNode;
      member->typeHash = _typeStack.top()->mangledNameHash;
      member->memberTypeHash = cppFunction->typeHash;
      member->kind = model::CppMemberType::Kind::Method;
      member->visibility = getMemberVisibility(md);
    }

    return true;
  }

  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl* cd_)
  {
    for (auto it = cd_->init_begin(); it != cd_->init_end(); ++it)
    {
      const clang::CXXCtorInitializer* init = *it;
      const clang::FieldDecl* member = init->getMember();

      if (!member || init->getSourceOrder() == -1)
        continue;

      model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

      astNode->astValue = member->getNameAsString();
      astNode->location = getFileLoc(
        init->getSourceRange().getBegin(),
        init->getSourceRange().getEnd());
      astNode->mangledName = getMangledName(_mngCtx, member);
      astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
      astNode->symbolType
        = isFunctionPointer(member)
        ? model::CppAstNode::SymbolType::FunctionPtr
        : model::CppAstNode::SymbolType::Variable;
      astNode->astType = model::CppAstNode::AstType::Write;

      astNode->id = model::createIdentifier(*astNode);

      if (insertToCache(member, astNode))
        _astNodes.push_back(astNode);
    }

    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* fd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = fd_->getNameAsString();
    astNode->location = getFileLoc(fd_->getLocStart(), fd_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, fd_);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType
      = isFunctionPointer(fd_)
      ? model::CppAstNode::SymbolType::FunctionPtr
      : model::CppAstNode::SymbolType::Variable;
    astNode->astType = model::CppAstNode::AstType::Definition;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(fd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppMemberType ---//

    model::CppMemberTypePtr member = std::make_shared<model::CppMemberType>();
    _members.push_back(member);

    clang::QualType qualType = fd_->getType();

    member->typeHash = _typeStack.top()->mangledNameHash;
    member->memberAstNode = astNode;
    member->memberTypeHash = util::fnvHash(getMangledName(_mngCtx, qualType));
    member->kind = model::CppMemberType::Kind::Field;
    member->visibility = getMemberVisibility(fd_);

    //--- CppVariable ---//

    model::CppVariablePtr variable = std::make_shared<model::CppVariable>();
    _variables.push_back(variable);

    variable->astNodeId = astNode->id;
    variable->mangledNameHash = astNode->mangledNameHash;
    variable->name = fd_->getNameAsString();
    variable->qualifiedName = fd_->getQualifiedNameAsString();
    variable->typeHash = member->memberTypeHash;
    variable->qualifiedType = qualType.getAsString();

    //--- CppAstNode for the type ---//

    model::CppAstNodePtr typeAstNode = std::make_shared<model::CppAstNode>();

    const clang::Type* type = getStrippedType(qualType);
    if (const clang::CXXRecordDecl* rd = type->getAsCXXRecordDecl())
    {
      typeAstNode->astValue = rd->getNameAsString();
      // The lexed token length will be added.
      typeAstNode->location = getFileLoc(
        fd_->getLocStart(),
        fd_->getLocStart());
      typeAstNode->mangledName = getMangledName(_mngCtx, rd);
      typeAstNode->mangledNameHash = util::fnvHash(typeAstNode->mangledName);
      typeAstNode->symbolType = model::CppAstNode::SymbolType::Type;
      typeAstNode->astType = model::CppAstNode::AstType::FieldTypeLoc;

      typeAstNode->id = model::createIdentifier(*typeAstNode);

      if (insertToCache(0, typeAstNode))
        _astNodes.push_back(typeAstNode);
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* vd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = vd_->getNameAsString();
    astNode->location = getFileLoc(vd_->getLocation(), vd_->getLocation());
    astNode->mangledName = getMangledName(_mngCtx, vd_, astNode->location);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType
      = isFunctionPointer(vd_)
      ? model::CppAstNode::SymbolType::FunctionPtr
      : model::CppAstNode::SymbolType::Variable;
    astNode->astType
      = vd_->isThisDeclarationADefinition() == clang::VarDecl::Definition
      ? model::CppAstNode::AstType::Definition
      : model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(vd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppAstNode for the type ---//

    model::CppAstNodePtr typeAstNode = std::make_shared<model::CppAstNode>();

    const clang::Type* type = getStrippedType(vd_->getType());
    if (const clang::CXXRecordDecl* rd = type->getAsCXXRecordDecl())
    {
      typeAstNode->astValue = rd->getNameAsString();
      typeAstNode->location = getFileLoc(
        vd_->getSourceRange().getBegin(),
        vd_->getSourceRange().getEnd());
      typeAstNode->mangledName = getMangledName(_mngCtx, rd);
      typeAstNode->mangledNameHash = util::fnvHash(typeAstNode->mangledName);
      typeAstNode->symbolType = model::CppAstNode::SymbolType::Type;
      typeAstNode->astType
        = _functionStack.empty()
        ? model::CppAstNode::AstType::GlobalTypeLoc
        : llvm::isa<clang::ParmVarDecl>(vd_)
        ? model::CppAstNode::AstType::ParameterTypeLoc
        : model::CppAstNode::AstType::LocalTypeLoc;

      typeAstNode->id = model::createIdentifier(*typeAstNode);

      // For compiler generated copy and move constructors have one parameter
      // with the same type without name. Since these parameters and their types
      // are invisible in the source code, all properties of the variable types
      // are the same. This is the reason why it is checked whether the cache
      // contains the non-existing AST node which belongs to the arguments'
      // type.
      if (insertToCache(0, typeAstNode))
        _astNodes.push_back(typeAstNode);
    }

    //--- CppVariable ---//

    model::CppVariablePtr variable = std::make_shared<model::CppVariable>();
    _variables.push_back(variable);

    clang::QualType qualType = vd_->getType();

    variable->astNodeId = astNode->id;
    variable->mangledNameHash = astNode->mangledNameHash;
    variable->name = vd_->getNameAsString();
    variable->qualifiedName = vd_->getQualifiedNameAsString();
    variable->typeHash = util::fnvHash(getMangledName(_mngCtx, qualType));
    variable->qualifiedType = qualType.getAsString();

    if (_functionStack.empty())
      variable->tags.insert(model::Tag::Global);

    //--- CppMemberType ---//

    if (!_typeStack.empty() && _functionStack.empty())
    {
      variable->tags.insert(model::Tag::Static);

      model::CppMemberTypePtr member = std::make_shared<model::CppMemberType>();
      _members.push_back(member);

      member->typeHash = _typeStack.top()->mangledNameHash;
      member->memberAstNode = astNode;
      member->memberTypeHash = variable->typeHash;
      member->kind = model::CppMemberType::Kind::Field;
      member->visibility = getMemberVisibility(vd_);
    }

    //--- Function parameters and local variables ---//

    // If the stack is empty, then it's a global variable.
    if (!_functionStack.empty())
    {
      if (llvm::isa<clang::ParmVarDecl>(vd_))
      {
        // TODO: This "if" is needed because the following example causes error:
        // void f1(void* (*f) (a));
        // void f2(void* (*f) (a));
        // For some reason these functions have 2 arguments, and the second one
        // is invalid and anonymous. The example causes error if "a" is not a
        // known type.
        if (astNode->location.range.start.line != -1u)
          _functionStack.top()->parameters.push_back(variable);
      }
      else
        _functionStack.top()->locals.push_back(variable);
    }

    return true;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl* nd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = nd_->getNameAsString();
    astNode->location = getFileLoc(nd_->getLocStart(), nd_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, nd_, astNode->location);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Namespace;
    astNode->astType = model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(nd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppNamespace ---//

    model::CppNamespacePtr ns = std::make_shared<model::CppNamespace>();
    _namespaces.push_back(ns);

    ns->astNodeId = astNode->id;
    ns->mangledNameHash = astNode->mangledNameHash;
    ns->name = nd_->getNameAsString();
    ns->qualifiedName = astNode->mangledName;

    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr* ce_)
  {
    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    const clang::CXXConstructorDecl* ctor = ce_->getConstructor();

    astNode->astValue = ctor->getNameAsString();
    astNode->location = getFileLoc(ce_->getLocStart(), ce_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, ctor);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType = model::CppAstNode::AstType::Usage;
    astNode->visibleInSourceCode = false;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ce_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitCXXNewExpr(clang::CXXNewExpr* ne_)
  {
    // TODO: Originally it was checked if getOperatorNew() returns
    // clang::DeclaratorDecl*, if this is a call through a function pointer.
    // According to the Clang reference guide getOperatorNew() always returns a
    // clang::FunctionDecl*.
    const clang::FunctionDecl* functionDecl = ne_->getOperatorNew();

    if (!functionDecl)
      return true;

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = functionDecl->getNameAsString();
    astNode->location = getFileLoc(ne_->getLocStart(), ne_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, functionDecl);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType = model::CppAstNode::AstType::Usage;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ne_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* de_)
  {
    // TODO: Originally it was checked if getOperatorNew() returns
    // clang::DeclaratorDecl*, if this is a call through a function pointer.
    // According to the Clang reference guide getOperatorNew() always returns a
    // clang::FunctionDecl*.
    const clang::FunctionDecl* functionDecl = de_->getOperatorDelete();

    if (!functionDecl)
      return true;

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = functionDecl->getNameAsString();
    astNode->location = getFileLoc(de_->getLocStart(), de_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, functionDecl);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType = model::CppAstNode::AstType::Usage;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(de_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitCallExpr(clang::CallExpr* ce_)
  {
    const clang::Decl* callee = ce_->getCalleeDecl();

    if (!callee) // TODO: How can this happen?
      return true;

    const clang::NamedDecl* namedCallee
      = llvm::dyn_cast<clang::NamedDecl>(callee);

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = namedCallee->getNameAsString();
    astNode->location = getFileLoc(ce_->getLocStart(), ce_->getLocEnd());
    astNode->mangledName = getMangledName(
      _mngCtx,
      namedCallee,
      getFileLoc(namedCallee->getLocStart(), namedCallee->getLocEnd()));
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType
      = isVirtualCall(ce_)
      ? model::CppAstNode::AstType::VirtualCall
      : model::CppAstNode::AstType::Usage;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ce_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* dr_)
  {
    const clang::ValueDecl* decl = dr_->getDecl();

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    if (const clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(decl))
    {
      astNode->astValue = vd->getNameAsString();
      astNode->location = getFileLoc(dr_->getLocStart(), dr_->getLocEnd());
      astNode->mangledName
        = getMangledName(_mngCtx, vd, astNode->location);
      astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
      astNode->symbolType
        = isFunctionPointer(vd)
        ? model::CppAstNode::SymbolType::FunctionPtr
        : model::CppAstNode::SymbolType::Variable;
      astNode->astType
        = isWritten(vd)
        ? model::CppAstNode::AstType::Write
        : model::CppAstNode::AstType::Read;

      astNode->id = model::createIdentifier(*astNode);
    }
    else if (const clang::EnumConstantDecl* ec
      = llvm::dyn_cast<clang::EnumConstantDecl>(decl))
    {
      astNode->astValue = ec->getNameAsString();
      astNode->location = getFileLoc(ec->getLocStart(), ec->getLocEnd());
      astNode->mangledName = getMangledName(_mngCtx, ec);
      astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
      astNode->symbolType = model::CppAstNode::SymbolType::EnumConstant;
      astNode->astType = model::CppAstNode::AstType::Usage;

      astNode->id = model::createIdentifier(*astNode);
    }
    else if (const clang::FunctionDecl* fd
      = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
      astNode->astValue = fd->getNameAsString();
      astNode->location = getFileLoc(fd->getLocStart(), fd->getLocEnd());
      astNode->mangledName = getMangledName(_mngCtx, fd);
      astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
      astNode->symbolType = model::CppAstNode::SymbolType::Function;
      astNode->astType = model::CppAstNode::AstType::Read;

      astNode->id = model::createIdentifier(*astNode);
    }

    if (insertToCache(dr_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitMemberExpr(clang::MemberExpr* me_)
  {
    const clang::ValueDecl* vd = me_->getMemberDecl();

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = vd->getNameAsString();
    astNode->location = getFileLoc(me_->getLocStart(), me_->getLocEnd());
    astNode->mangledName = getMangledName(_mngCtx, vd);
    astNode->mangledNameHash = util::fnvHash(astNode->mangledName);
    astNode->symbolType
      = llvm::isa<clang::CXXMethodDecl>(vd)
      ? model::CppAstNode::SymbolType::Function
      : isFunctionPointer(vd)
      ? model::CppAstNode::SymbolType::FunctionPtr
      : model::CppAstNode::SymbolType::Variable;
    astNode->astType
      = isWritten(llvm::dyn_cast<clang::VarDecl>(vd))
      ? model::CppAstNode::AstType::Write
      : model::CppAstNode::AstType::Read;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(me_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitCXXMethodDecl(clang::CXXMethodDecl* decl)
  {
    for (auto it = decl->begin_overridden_methods();
         it != decl->end_overridden_methods();
         ++it)
    {
      auto left = _clangToAstNodeId.find(decl);
      auto right = _clangToAstNodeId.find(*it);

      if (left == _clangToAstNodeId.end() || right == _clangToAstNodeId.end())
        continue;

      model::CppRelationPtr rel = std::make_shared<model::CppRelation>();
      rel->kind = model::CppRelation::Kind::Override;
      rel->lhs = _mangledNameCache.at(left->second);
      rel->rhs = _mangledNameCache.at(right->second);
      _relations.push_back(rel);
    }

    return true;
  }

private:
  /**
   * This function inserts a model::CppAstNodeId to a cache in a thread-safe
   * way. The cache is static so the parsers in each thread can use the same.
   * Moreover the AST node ID will be mapped to the pointer which identifies
   * the clang AST node.
   *
   * @return If the insertion was successful (i.e. the cache didn't contain the
   * id before) then the function returns true.
   */
  bool insertToCache(const void* clangPtr_, model::CppAstNodePtr node_)
  {
    _clangToAstNodeId[clangPtr_] = node_->id;
    return _mangledNameCache.insert(*node_);
  }

  model::FileLoc getFileLoc(
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_)
  {
    model::FileLoc fileLoc;

    if (start_.isInvalid() || end_.isInvalid())
    {
      fileLoc.file = _ctx.srcMgr.getFile(_fileLocUtil.getFilePath(start_));
      const std::string& type = fileLoc.file.load()->type;
      if (type != model::File::DIRECTORY_TYPE && type != _cppSourceType)
      {
        fileLoc.file->type = _cppSourceType;
        _ctx.srcMgr.updateFile(*fileLoc.file);
      }
      return fileLoc;
    }

    clang::SourceLocation realStart = start_;
    clang::SourceLocation realEnd = end_;

    if (_clangSrcMgr.isMacroArgExpansion(start_))
      realStart = _clangSrcMgr.getSpellingLoc(start_);
    if (_clangSrcMgr.isMacroArgExpansion(end_))
      realEnd = _clangSrcMgr.getSpellingLoc(end_);

    if (!_isImplicit)
      _fileLocUtil.setRange(realStart, realEnd, fileLoc.range);

    fileLoc.file = _ctx.srcMgr.getFile(_fileLocUtil.getFilePath(realStart));

    const std::string& type = fileLoc.file.load()->type;
    if (type != model::File::DIRECTORY_TYPE && type != _cppSourceType)
    {
      fileLoc.file->type = _cppSourceType;
      _ctx.srcMgr.updateFile(*fileLoc.file);
    }

    return fileLoc;
  }

  bool isFunctionPointer(const clang::ValueDecl* vd_) const
  {
    const clang::Type* type = vd_->getType().getTypePtrOrNull();

    while (type)
      if (type->isFunctionType() ||
          type->isFunctionPointerType() ||
          type->isMemberFunctionPointerType())
        return true;
      else if (auto pType = type->getAs<clang::ParenType>())
        type = pType->getInnerType().getTypePtrOrNull();
      else if (auto aType = llvm::dyn_cast<clang::ArrayType>(type))
        type = aType->getElementType().getTypePtrOrNull();
      else if (type->isPointerType() || type->isReferenceType())
        type = type->getPointeeType().getTypePtrOrNull();
      else if (auto eType = llvm::dyn_cast<clang::ElaboratedType>(type))
        type = eType->desugar().getTypePtrOrNull();
      else
        return false;

    return false;
  }

  bool isVirtualCall(const clang::CallExpr* ce_) const
  {
    //--- If not a method is called then return false ---//

    const clang::CXXMemberCallExpr* memberCallExpr
      = llvm::dyn_cast<clang::CXXMemberCallExpr>(ce_);

    if (!memberCallExpr)
      return false;

    //--- If the method is not virtual then return false ---//

    const clang::CXXMethodDecl* method = memberCallExpr->getMethodDecl();

    if (!method || !method->isVirtual())
      return false;

    //--- Check the object on which the method is called ---//

    const clang::Expr* obj = memberCallExpr->getImplicitObjectArgument();

    if (!obj)
      return false;

    //--- If the object is a pointer then return true ---//

    const clang::Type* type = obj->getType().getTypePtrOrNull();

    if (type && type->isPointerType())
      return true;

    //--- If the object is a reference then return true ---//

    if (auto declRef = llvm::dyn_cast<clang::DeclRefExpr>(obj))
      if (auto decl = declRef->getDecl())
        if (auto vDecl = llvm::dyn_cast<clang::VarDecl>(decl))
          if (auto vDeclType = vDecl->getType().getTypePtr())
            if (vDeclType->isReferenceType())
              return true;

    return false;
  }

  model::Visibility getVisibility(clang::AccessSpecifier access_) const
  {
    switch (access_)
    {
      case clang::AS_public:    return model::Visibility::Public;
      case clang::AS_protected: return model::Visibility::Protected;
      case clang::AS_private:   return model::Visibility::Private;
      case clang::AS_none:      return model::Visibility::Public;
    }

    return model::Visibility::Public;
  }

  model::Visibility getMemberVisibility(const clang::Decl* decl_) const
  {
    return getVisibility(decl_->getAccess());
  }

  bool isWritten(const clang::VarDecl* vd_) const
  {
    // TODO
    return true;
  }

  // TODO: This should be in the model.
  template <typename Cont>
  void persistAll(Cont& cont_)
  {
    for (typename Cont::value_type& item : cont_)
    {
      try
      {
        _ctx.db->persist(*item);
      }
      catch (const odb::object_already_persistent& ex)
      {
        LOG(debug)
          << item->toString();
        LOG(warning)
          << ex.what() << std::endl
          << "AST nodes in this translation unit will be ignored!";
      }
      catch (const odb::database_exception& ex)
      {
        // TODO: Error code should be checked and rethrow if it is not unique
        // constraint error. Error code may be database specific.
      }
    }
  }

  std::vector<model::CppAstNodePtr>      _astNodes;
  std::vector<model::CppEnumConstantPtr> _enumConstants;
  std::vector<model::CppEnumPtr>         _enums;
  std::vector<model::CppFunctionPtr>     _functions;
  std::vector<model::CppTypePtr>         _types;
  std::vector<model::CppTypedefPtr>      _typedefs;
  std::vector<model::CppVariablePtr>     _variables;
  std::vector<model::CppNamespacePtr>    _namespaces;
  std::vector<model::CppMemberTypePtr>   _members;
  std::vector<model::CppInheritancePtr>  _inheritances;
  std::vector<model::CppFriendshipPtr>   _friends;
  std::vector<model::CppRelationPtr>     _relations;

  // TODO: Maybe we don't even need a stack, if functions can't be nested.
  // Check lambda.
  // TODO: _enumStack also doesn't have to be a stack.
  std::stack<model::CppFunctionPtr> _functionStack;
  std::stack<model::CppTypePtr>     _typeStack;
  std::stack<model::CppEnumPtr>     _enumStack;

  bool _isImplicit;
  ParserContext& _ctx;
  const clang::SourceManager& _clangSrcMgr;
  FileLocUtil _fileLocUtil;
  clang::MangleContext* _mngCtx;
  const std::string _cppSourceType;

  MangledNameCache& _mangledNameCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;
};

}
}

#endif
