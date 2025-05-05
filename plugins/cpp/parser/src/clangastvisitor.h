#ifndef CC_PARSER_CLANGASTVISITOR_H
#define CC_PARSER_CLANGASTVISITOR_H

#include <memory>
#include <mutex>
#include <type_traits>
#include <stack>
#include <functional>

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ParentMapContext.h>
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
#include <model/cppnamespacealias.h>
#include <model/cppnamespacealias-odb.hxx>
#include <model/cpprelation.h>
#include <model/cpprelation-odb.hxx>
#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>
#include <model/cpptypedef.h>
#include <model/cpptypedef-odb.hxx>

#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>
#include <util/hash.h>
#include <util/odbtransaction.h>
#include <util/scopedvalue.h>

#include <cppparser/filelocutil.h>

#include "entitycache.h"
#include "symbolhelper.h"
#include "nestedscope.h"

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
  using Base = clang::RecursiveASTVisitor<ClangASTVisitor>;

  ClangASTVisitor(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    EntityCache& entityCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
    : _isImplicit(false),
      _ctx(ctx_),
      _clangSrcMgr(astContext_.getSourceManager()),
      _fileLocUtil(astContext_.getSourceManager()),
      _astContext(astContext_),
      _mngCtx(astContext_.createMangleContext()),
      _cppSourceType("CPP"),
      _entityCache(entityCache_),
      _clangToAstNodeId(clangToAstNodeId_)
  {
  }

  ~ClangASTVisitor()
  {
    _ctx.srcMgr.persistFiles();

    for (auto& p : _locToTypeLoc)
    {
      model::CppAstNodePtr typeLocAstNode = p.second;

      auto itAstType = _locToAstType.find(p.first);

      if (itAstType != _locToAstType.end())
        typeLocAstNode->astType = itAstType->second;

      auto itAstValue = _locToAstValue.find(p.first);

      if (itAstValue != _locToAstValue.end())
        typeLocAstNode->astValue = itAstValue->second;

      typeLocAstNode->id = createIdentifier(*typeLocAstNode);

      if (insertToCache(0, typeLocAstNode))
        _astNodes.push_back(typeLocAstNode);
    }

    (util::OdbTransaction(_ctx.db))([this]{
      util::persistAll(_astNodes, _ctx.db);
      util::persistAll(_enumConstants, _ctx.db);
      util::persistAll(_enums, _ctx.db);
      util::persistAll(_types, _ctx.db);
      util::persistAll(_typedefs, _ctx.db);
      util::persistAll(_variables, _ctx.db);
      util::persistAll(_namespaces, _ctx.db);
      util::persistAll(_namespaceAliases, _ctx.db);
      util::persistAll(_members, _ctx.db);
      util::persistAll(_inheritances, _ctx.db);
      util::persistAll(_friends, _ctx.db);
      util::persistAll(_functions, _ctx.db);
      util::persistAll(_relations, _ctx.db);
    });
  }


  // AST Visitor configuration

  bool shouldVisitImplicitCode() const { return true; }
  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitLambdaBody() const { return true; }
  

  // Traverse Decl helpers

  class TypeScope final
  {
    DECLARE_SCOPED_TYPE(TypeScope)
    
  private:
    ClangASTVisitor* _visitor;
    model::CppRecordPtr _type;

  public:
    TypeScope(
      ClangASTVisitor* visitor_
    ) :
      _visitor(visitor_),
      _type(std::make_shared<model::CppRecord>())
    {
      _visitor->_typeStack.push(_type);
    }

    ~TypeScope()
    {
      assert(_type == _visitor->_typeStack.top() &&
        "Type scope destruction order has been violated.");
      if (_type->astNodeId)
        _visitor->_types.push_back(_type);
      _visitor->_typeStack.pop();
    }
  };

  class EnumScope final
  {
    DECLARE_SCOPED_TYPE(EnumScope)

  private:
    ClangASTVisitor* _visitor;
    model::CppEnumPtr _enum;

  public:
    EnumScope(
      ClangASTVisitor* visitor_
    ) :
      _visitor(visitor_),
      _enum(std::make_shared<model::CppEnum>())
    {
      _visitor->_enumStack.push(_enum);
    }

    ~EnumScope()
    {
      assert(_enum == _visitor->_enumStack.top() &&
        "Enum scope destruction order has been violated.");
      if (_enum->astNodeId)
        _visitor->_enums.push_back(_enum);
      _visitor->_enumStack.pop();
    }
  };

  class FunctionScope final
  {
    DECLARE_SCOPED_TYPE(FunctionScope)

  private:
    ClangASTVisitor* _visitor;
    model::CppFunctionPtr _curFun;

  public:
    FunctionScope(ClangASTVisitor* visitor_) :
      _visitor(visitor_),
      _curFun(std::make_shared<model::CppFunction>())
    {
      _visitor->_functionStack.push(_curFun);
    }

    ~FunctionScope()
    {
      assert(_curFun == _visitor->_functionStack.top() &&
        "Function scope destruction order has been violated.");
      if (_curFun->astNodeId)
        _visitor->_functions.push_back(_curFun);
      _visitor->_functionStack.pop();

      if (_curFun->astNodeId && !_visitor->_functionStack.empty())
      {
        StatementScope* scope = _visitor->_stmtStack.TopValid();
        assert(scope != nullptr &&
          "Previous function entry has no corresponding scope stack entry.");
        scope->EnsureConfigured();

        // If the currently parsed function had a total bumpiness of B
        // from S statements, and is nested inside an enclosing function
        // at depth D, then the total bumpiness of that enclosing function
        // must be increased by the total bumpiness of the nested function:
        // B (core nested bumpiness) + S * D (accounting for the indentation).
        model::CppFunctionPtr& prevFun = _visitor->_functionStack.top();
        prevFun->bumpiness += _curFun->bumpiness +
          _curFun->statementCount * scope->Depth();
        prevFun->statementCount += _curFun->statementCount;
      }
    }
  };


  // Traverse Decl

  bool TraverseCXXRecordDecl(clang::CXXRecordDecl* rd_)
  {
    // Although lambda closure types are implicit by nature as far as
    // Clang is concerned, their operator() is not. In order to be able to
    // properly assign symbol location information to AST nodes within
    // lambda bodies, we must force lambdas to be considered explicit.
    util::ScopedValue<bool> sv(_isImplicit, _isImplicit && !rd_->isLambda());
    return Base::TraverseCXXRecordDecl(rd_);
  }

  bool TraverseEnumDecl(clang::EnumDecl* ed_)
  {
    // Any type of enum must be placed on the enum stack
    // for the duration of its traversal.
    // The scope creates a database object for the enum
    // at the beginning, and persists it at the end.
    EnumScope es(this);
    return Base::TraverseEnumDecl(ed_);
  }


  bool TraverseDecl(clang::Decl* d_)
  {
    if (d_ == nullptr)
      return Base::TraverseDecl(d_);

    // We use implicitness to determine if actual symbol location information
    // should be stored for AST nodes in our database. This differs somewhat
    // from Clang's concept of implicitness.
    // To bridge the gap between the two interpretations, we mostly assume
    // implicitness to be hereditary from parent to child nodes, except
    // in some known special cases (see lambdas in TraverseCXXRecordDecl).
    util::ScopedValue<bool> sv(_isImplicit, _isImplicit || d_->isImplicit());
    
    if (clang::FunctionDecl* fd = llvm::dyn_cast<clang::FunctionDecl>(d_))
    {
      // Any type of function must be placed on the function stack
      // for the duration of its traversal.
      // The scope creates a database object for the function
      // at the beginning, and persists it at the end.
      FunctionScope fs(this);
      // We must also make an initial scope entry for the function's body.
      StatementScope ss(&_stmtStack, nullptr);
      ss.MakeFunction(fd->getBody());
      return Base::TraverseDecl(fd);
    }
    else if (clang::RecordDecl* rd = llvm::dyn_cast<clang::RecordDecl>(d_))
    {
      // Any type of record/type must be placed on the type stack
      // for the duration of its traversal.
      // The scope creates a database object for the record/type
      // at the beginning, and persists it at the end.
      TypeScope ts(this);
      return Base::TraverseDecl(rd);
    }
    else return Base::TraverseDecl(d_);
  }


  // Metrics helpers

  void CountMcCabe()
  {
    if (!_functionStack.empty())
      ++_functionStack.top()->mccabe;
  }

  void CountBumpiness(const StatementScope& scope_)
  {
    if (!_functionStack.empty() && scope_.IsReal())
    {
      model::CppFunctionPtr& fun = _functionStack.top();
      fun->bumpiness += scope_.Depth();
      ++fun->statementCount;
    }
  }


  // Traverse Stmt helpers

  class CtxStmtScope final
  {
    DECLARE_SCOPED_TYPE(CtxStmtScope)

  private:
    ClangASTVisitor* _visitor;

  public:
    CtxStmtScope(
      ClangASTVisitor* visitor_,
      clang::Stmt* stmt_
    ) :
      _visitor(visitor_)
    {
      _visitor->_contextStatementStack.push(stmt_);
    }

    ~CtxStmtScope()
    {
      _visitor->_contextStatementStack.pop();
    }
  };


  // Traverse Expr (Expr : Stmt)

  bool TraverseCallExpr(clang::CallExpr* ce_)
  {
    CtxStmtScope css(this, ce_);
    return Base::TraverseCallExpr(ce_);
  }

  bool TraverseMemberExpr(clang::MemberExpr* me_)
  {
    CtxStmtScope css(this, me_);
    return Base::TraverseMemberExpr(me_);
  }

  bool TraverseCXXDeleteExpr(clang::CXXDeleteExpr* de_)
  {
    CtxStmtScope css(this, de_);
    return Base::TraverseCXXDeleteExpr(de_);
  }

  bool TraverseBinaryOperator(clang::BinaryOperator* bo_)
  {
    if (bo_->isLogicalOp()) CountMcCabe();
    CtxStmtScope css(this, bo_);
    return Base::TraverseBinaryOperator(bo_);
  }

  bool TraverseConditionalOperator(clang::ConditionalOperator* co_)
  {
    CountMcCabe();
    return Base::TraverseConditionalOperator(co_);
  }

  bool TraverseBinaryConditionalOperator(
    clang::BinaryConditionalOperator* bco_)
  {
    CountMcCabe();
    return Base::TraverseBinaryConditionalOperator(bco_);
  }


  // Traverse Stmt

  bool TraverseCompoundStmt(clang::CompoundStmt* cs_)
  {
    _stmtStack.Top()->MakeCompound();
    return Base::TraverseCompoundStmt(cs_);
  }

  bool TraverseDeclStmt(clang::DeclStmt* ds_)
  {
    CtxStmtScope css(this, ds_);
    return Base::TraverseDeclStmt(ds_);
  }

  bool TraverseReturnStmt(clang::ReturnStmt* rs_)
  {
    CtxStmtScope css(this, rs_);
    return Base::TraverseReturnStmt(rs_);
  }

  bool TraverseIfStmt(clang::IfStmt* is_)
  {
    _stmtStack.Top()->MakeTwoWay(is_->getThen(), is_->getElse());
    CountMcCabe();
    return Base::TraverseIfStmt(is_);
  }

  bool TraverseWhileStmt(clang::WhileStmt* ws_)
  {
    _stmtStack.Top()->MakeOneWay(ws_->getBody());
    CountMcCabe();
    return Base::TraverseWhileStmt(ws_);
  }

  bool TraverseDoStmt(clang::DoStmt* ds_)
  {
    _stmtStack.Top()->MakeOneWay(ds_->getBody());
    CountMcCabe();
    return Base::TraverseDoStmt(ds_);
  }

  bool TraverseForStmt(clang::ForStmt* fs_)
  {
    _stmtStack.Top()->MakeOneWay(fs_->getBody());
    CountMcCabe();
    return Base::TraverseForStmt(fs_);
  }

  bool TraverseCXXForRangeStmt(clang::CXXForRangeStmt* frs_)
  {
    _stmtStack.Top()->MakeOneWay(frs_->getBody());
    CountMcCabe();
    return Base::TraverseCXXForRangeStmt(frs_);
  }

  bool TraverseSwitchStmt(clang::SwitchStmt* ss_)
  {
    _stmtStack.Top()->MakeOneWay(ss_->getBody());
    return Base::TraverseSwitchStmt(ss_);
  }

  bool TraverseCaseStmt(clang::CaseStmt* cs_)
  {
    _stmtStack.Top()->MakeTransparent();
    CountMcCabe();
    return Base::TraverseCaseStmt(cs_);
  }

  bool TraverseDefaultStmt(clang::DefaultStmt* ds_)
  {
    _stmtStack.Top()->MakeTransparent();
    CountMcCabe();
    return Base::TraverseDefaultStmt(ds_);
  }

  bool TraverseContinueStmt(clang::ContinueStmt* cs_)
  {
    CountMcCabe();
    return Base::TraverseContinueStmt(cs_);
  }

  bool TraverseGotoStmt(clang::GotoStmt* gs_)
  {
    CountMcCabe();
    return Base::TraverseGotoStmt(gs_);
  }

  bool TraverseCXXTryStmt(clang::CXXTryStmt* ts_)
  {
    _stmtStack.Top()->MakeOneWay(ts_->getTryBlock());
    return Base::TraverseCXXTryStmt(ts_);
  }

  bool TraverseCXXCatchStmt(clang::CXXCatchStmt* cs_)
  {
    _stmtStack.Top()->MakeOneWay(cs_->getHandlerBlock());
    CountMcCabe();
    return Base::TraverseCXXCatchStmt(cs_);
  }


  bool TraverseStmt(clang::Stmt* s_)
  {
    if (s_ == nullptr)
      return Base::TraverseStmt(s_);

    // Create a statement scope for the current statement for the duration
    // of its traversal. For every statement, there must be exactly one scope.
    StatementScope scope(&_stmtStack, s_);
    bool b = Base::TraverseStmt(s_);
    // Base::TraverseStmt will select the best handler function for the
    // statement's dynamic type. If it is not any of the special statement
    // types we are explicitly handling, it must be a "normal" statement.
    // In that case, none of the scope's specialized Make* member functions
    // will be called. Even so, before counting its nestedness,
    // we must ensure it is configured at least as a normal statement.
    scope.EnsureConfigured();
    CountBumpiness(scope);
    return b;
  }


   // Visit functions

  bool VisitTypedefTypeLoc(clang::TypedefTypeLoc tl_)
  {
    const clang::TypedefType* type = tl_.getTypePtr();
    if (!type) return true;

    const clang::TypedefNameDecl* td = type->getDecl();

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->location = getFileLoc(tl_.getBeginLoc(), tl_.getEndLoc());
    astNode->astType = model::CppAstNode::AstType::TypeLocation;
    astNode->astValue = td->getNameAsString();
    astNode->symbolType = model::CppAstNode::SymbolType::Typedef;
    astNode->entityHash = util::fnvHash(getUSR(td));

    _locToTypeLoc[tl_.getBeginLoc().getRawEncoding()] = astNode;

    return true;
  }

  bool VisitEnumTypeLoc(clang::EnumTypeLoc tl_)
  {
    const clang::EnumType* type = tl_.getTypePtr();
    if (!type) return true;

    const clang::EnumDecl* ed = type->getDecl();

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->location = getFileLoc(tl_.getBeginLoc(), tl_.getEndLoc());
    astNode->astType = model::CppAstNode::AstType::TypeLocation;
    astNode->astValue = ed->getNameAsString();
    astNode->symbolType = model::CppAstNode::SymbolType::Enum;
    astNode->entityHash = util::fnvHash(getUSR(ed));

    _locToTypeLoc[tl_.getBeginLoc().getRawEncoding()] = astNode;

    return true;
  }

  bool VisitRecordTypeLoc(clang::RecordTypeLoc tl_)
  {
    const clang::RecordType* type = tl_.getTypePtr();
    if (!type) return true;

    const clang::RecordDecl* rd = type->getDecl();

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->location = getFileLoc(tl_.getBeginLoc(), tl_.getEndLoc());
    astNode->astType = model::CppAstNode::AstType::TypeLocation;
    astNode->astValue = rd->getNameAsString();
    astNode->symbolType = model::CppAstNode::SymbolType::Type;
    astNode->entityHash = util::fnvHash(getUSR(rd));

    _locToTypeLoc[tl_.getBeginLoc().getRawEncoding()] = astNode;

    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* rd_)
  {
    // The subtree of a CXXRecordDecl node contains another CXXRecordDecl for
    // some implementation defined reasons. The position of this node is at the
    // name of the record after the "class", "struct", etc. keywords. We don't
    // want to store these in the database
    if (rd_->isImplicit() && !rd_->isLambda())
      return true;

    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getDeclPartAsString(_clangSrcMgr, rd_);
    astNode->location = getFileLoc(rd_->getBeginLoc(), rd_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(rd_));
    astNode->symbolType = model::CppAstNode::SymbolType::Type;
    astNode->astType
      = rd_->isThisDeclarationADefinition()
      ? model::CppAstNode::AstType::Definition
      : model::CppAstNode::AstType::Declaration;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(rd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppRecord ---//

    if (!rd_->isThisDeclarationADefinition())
      return true;

    model::CppRecordPtr cppRecord = _typeStack.top();

    cppRecord->astNodeId = astNode->id;
    cppRecord->entityHash = astNode->entityHash;
    cppRecord->name = rd_->getNameAsString();
    cppRecord->qualifiedName = rd_->getQualifiedNameAsString();

    if (const clang::CXXRecordDecl* crd
      = llvm::dyn_cast<clang::CXXRecordDecl>(rd_))
    {
      cppRecord->isAbstract = crd->isAbstract();
      cppRecord->isPOD = crd->isPOD();

      if (crd->getTemplateInstantiationPattern())
      {
        cppRecord->tags.insert(
          crd->getTemplateSpecializationKind() == clang::TSK_ExplicitSpecialization
          ? model::Tag::TemplateSpecialization
          : model::Tag::TemplateInstantiation
        );
      }

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

          inheritance->derived = cppRecord->entityHash;
          inheritance->base = util::fnvHash(getUSR(baseDecl));
          inheritance->isVirtual = it->isVirtual();
          inheritance->visibility = getVisibility(it->getAccessSpecifier());

          //--- AST type for inherited class ---//

          unsigned rawEncoding = it->getBaseTypeLoc().getRawEncoding();
          _locToAstType[rawEncoding]
            = model::CppAstNode::AstType::InheritanceTypeLoc;
          _locToAstValue[rawEncoding]
            = getDeclPartAsString(_clangSrcMgr, crd);
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

          friendship->target = cppRecord->entityHash;
          friendship->theFriend = util::fnvHash(getUSR(cxxRecordDecl));

          clang::SourceRange range = (*it)->getSourceRange();
          _locToAstValue[tsi->getTypeLoc().getBeginLoc().getRawEncoding()]
            = getSourceText(_clangSrcMgr,
                range.getBegin(), range.getEnd(), true);
        }
        else if (clang::NamedDecl* friendDecl = (*it)->getFriendDecl())
        {
          //--- Friend functions ---//

          model::CppFriendshipPtr friendship
            = std::make_shared<model::CppFriendship>();
          _friends.push_back(friendship);

          friendship->target = cppRecord->entityHash;
          friendship->theFriend = util::fnvHash(getUSR(friendDecl));
        }
      }

      // --- Destructor --- //

      const clang::NamedDecl* ref = crd;
      model::FileLoc location = getFileLoc(crd->getEndLoc(), crd->getEndLoc());
      if (clang::CXXDestructorDecl* dd = crd->getDestructor())
      {
        ref = dd;
        if (crd->hasUserDeclaredDestructor())
        {
          clang::Stmt* body = dd->getBody();
          location = body
            ? getFileLoc(body->getEndLoc(), body->getEndLoc())
            : getFileLoc(dd->getEndLoc(), dd->getEndLoc());
        }
      }

      for (auto it = crd->field_begin(); it != crd->field_end(); ++it)
      {
        clang::FieldDecl* fd = *it;
        addDestructorUsage(fd->getType(), location, ref);
      }
    }

    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* ed_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getDeclPartAsString(_clangSrcMgr, ed_);
    astNode->location = getFileLoc(ed_->getBeginLoc(), ed_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(ed_));
    astNode->symbolType = model::CppAstNode::SymbolType::Enum;
    astNode->astType
      = ed_->isThisDeclarationADefinition()
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
    cppEnum->entityHash = astNode->entityHash;
    cppEnum->name = name;
    cppEnum->qualifiedName = ed_->getQualifiedNameAsString();

    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl* ec_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = ec_->getNameAsString();
    astNode->location = getFileLoc(ec_->getBeginLoc(), ec_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(ec_));
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
    enumConstant->entityHash = astNode->entityHash;
    enumConstant->name = ec_->getNameAsString();
    enumConstant->qualifiedName = ec_->getQualifiedNameAsString();
    enumConstant->value = ec_->getInitVal().getLimitedValue();

    _enumStack.top()->enumConstants.push_back(enumConstant);

    return true;
  }

  bool VisitTypedefNameDecl(clang::TypedefNameDecl* td_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getSourceText(
      _clangSrcMgr,
      td_->getSourceRange().getBegin(),
      td_->getSourceRange().getEnd(),
      true);
    astNode->location = getFileLoc(td_->getBeginLoc(), td_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(td_));
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
    cppTypedef->entityHash = astNode->entityHash;
    cppTypedef->name = td_->getNameAsString();
    cppTypedef->qualifiedName = td_->getQualifiedNameAsString();
    cppTypedef->typeHash = util::fnvHash(getUSR(qualType, _astContext));
    cppTypedef->qualifiedType = qualType.getAsString();

    //--- AST type for aliased type ---//

    const clang::Type* type = getStrippedType(qualType);
    if (type->isRecordType() || type->isEnumeralType())
    {
      clang::TypeLoc loc = td_->getTypeSourceInfo()->getTypeLoc();

      unsigned rawEncoding = loc.getBeginLoc().getRawEncoding();
      _locToAstType[rawEncoding] = model::CppAstNode::AstType::TypedefTypeLoc;
      _locToAstValue[rawEncoding] = astNode->astValue;
    }

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* fn_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getSignature(fn_);
    astNode->location = getFileLoc(fn_->getBeginLoc(), fn_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(fn_));
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
    cppFunction->entityHash = astNode->entityHash;
    cppFunction->name = fn_->getNameAsString();
    cppFunction->qualifiedName = fn_->getQualifiedNameAsString();
    cppFunction->typeHash = util::fnvHash(getUSR(qualType, _astContext));
    cppFunction->qualifiedType = qualType.getAsString();
    cppFunction->mccabe = fn_->isThisDeclarationADefinition() ? 1 : 0;
    cppFunction->bumpiness = 0;
    cppFunction->statementCount = 0;

    //--- Tags ---//

    if (_isImplicit)
      cppFunction->tags.insert(model::Tag::Implicit);

    clang::CXXMethodDecl* md = llvm::dyn_cast<clang::CXXMethodDecl>(fn_);
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

    //--- AST type for the return type ---//

    unsigned rawEncoding
      = fn_->getReturnTypeSourceRange().getBegin().getRawEncoding();
    _locToAstType[rawEncoding] = model::CppAstNode::AstType::ReturnTypeLoc;
    _locToAstValue[rawEncoding] = astNode->astValue;

    //--- CppMemberType ---//

    // If empty then this is a global declaration.
    if (md && !_typeStack.empty())
    {
      model::CppMemberTypePtr member
        = std::make_shared<model::CppMemberType>();
      _members.push_back(member);

      member->memberAstNode = astNode;
      member->typeHash = _typeStack.top()->entityHash;
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

      astNode->astValue = getSignature(cd_);
      astNode->location = getFileLoc(
        init->getSourceRange().getBegin(),
        init->getSourceRange().getEnd());
      astNode->entityHash = util::fnvHash(getUSR(member));
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

    astNode->astValue = fd_->getType().getAsString();
    astNode->astValue.append(" ");
    astNode->astValue.append(fd_->getNameAsString());

    astNode->location = getFileLoc(fd_->getBeginLoc(), fd_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(fd_));
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

    member->typeHash = _typeStack.top()->entityHash;
    member->memberAstNode = astNode;
    member->memberTypeHash = util::fnvHash(getUSR(qualType, _astContext));
    member->kind = model::CppMemberType::Kind::Field;
    member->visibility = getMemberVisibility(fd_);

    //--- CppVariable ---//

    model::CppVariablePtr variable = std::make_shared<model::CppVariable>();
    _variables.push_back(variable);

    variable->astNodeId = astNode->id;
    variable->entityHash = astNode->entityHash;
    variable->name = fd_->getNameAsString();
    variable->qualifiedName = fd_->getQualifiedNameAsString();
    variable->typeHash = member->memberTypeHash;
    variable->qualifiedType = qualType.getAsString();

    //--- AST type for the type ---//

    unsigned rawEncoding = fd_->getTypeSpecStartLoc().getRawEncoding();
    _locToAstType[rawEncoding]
      = astNode->symbolType == model::CppAstNode::SymbolType::FunctionPtr
      ? model::CppAstNode::AstType::ReturnTypeLoc
      : model::CppAstNode::AstType::FieldTypeLoc;
    _locToAstValue[rawEncoding] = astNode->astValue;

    return true;
  }

  bool VisitVarDecl(clang::VarDecl* vd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = vd_->getType().getAsString();
    astNode->astValue.append(" ");
    astNode->astValue.append(vd_->getNameAsString());

    astNode->location = getFileLoc(vd_->getBeginLoc(), vd_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(vd_));
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

    //--- AST type for the type ---//

    unsigned rawEncoding = vd_->getTypeSpecStartLoc().getRawEncoding();
    _locToAstType[rawEncoding]
      // The parameter part of a function pointer declaration is considered a
      // parameter type location.
      = llvm::isa<clang::ParmVarDecl>(vd_)
      ? model::CppAstNode::AstType::ParameterTypeLoc
      : _functionStack.empty()
      ? model::CppAstNode::AstType::GlobalTypeLoc
      : model::CppAstNode::AstType::LocalTypeLoc;
    _locToAstValue[rawEncoding] = astNode->astValue;

    //--- CppVariable ---//

    model::CppVariablePtr variable = std::make_shared<model::CppVariable>();
    _variables.push_back(variable);

    clang::QualType qualType = vd_->getType();

    variable->astNodeId = astNode->id;
    variable->entityHash = astNode->entityHash;
    variable->name = vd_->getNameAsString();
    variable->qualifiedName = vd_->getQualifiedNameAsString();
    variable->typeHash = util::fnvHash(getUSR(qualType, _astContext));
    variable->qualifiedType = qualType.getAsString();

    if (_functionStack.empty())
      variable->tags.insert(model::Tag::Global);
    if (_isImplicit)
      variable->tags.insert(model::Tag::Implicit);

    //--- CppMemberType ---//

    if (!_typeStack.empty() && _functionStack.empty())
    {
      variable->tags.insert(model::Tag::Static);

      model::CppMemberTypePtr member = std::make_shared<model::CppMemberType>();
      _members.push_back(member);

      member->typeHash = _typeStack.top()->entityHash;
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

  bool VisitDeclStmt(clang::DeclStmt* ds_)
  {
    clang::Stmt* s;
    StatementScope* scope = _stmtStack.TopValid();
    if (scope->Statement() == ds_)
    {
      assert(scope->Previous() != nullptr &&
        "Declaration statement is not nested in a scope.");
      s = scope->Previous()->Statement();
    }
    else
      s = scope->Statement();

    for (auto it = ds_->decl_begin(); it != ds_->decl_end(); ++it)
    {
      if (clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(*it))
      {
        model::FileLoc loc =
          getFileLoc(s->getEndLoc(), s->getEndLoc());
        addDestructorUsage(vd->getType(), loc, vd);
      }
    }

    return true;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl* nd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getSourceText(
      _clangSrcMgr,
      nd_->getBeginLoc(),
      nd_->getLocation(),
      true);
    std::string usr = getUSR(nd_);
    astNode->location = getFileLoc(nd_->getBeginLoc(), nd_->getEndLoc());
    astNode->entityHash = util::fnvHash(usr);
    astNode->symbolType = model::CppAstNode::SymbolType::Namespace;
    astNode->astType = model::CppAstNode::AstType::Definition;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(nd_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppNamespace ---//

    model::CppNamespacePtr ns = std::make_shared<model::CppNamespace>();
    _namespaces.push_back(ns);

    ns->astNodeId = astNode->id;
    ns->entityHash = astNode->entityHash;
    ns->name = nd_->getNameAsString();
    ns->qualifiedName = usr;

    return true;
  }

  bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* nad_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = getSourceText(
      _clangSrcMgr,
      nad_->getBeginLoc(),
      nad_->getLocation(),
      true);

    astNode->location = getFileLoc(nad_->getBeginLoc(), nad_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(nad_->getAliasedNamespace()));
    astNode->symbolType = model::CppAstNode::SymbolType::NamespaceAlias;
    astNode->astType = model::CppAstNode::AstType::Definition;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(nad_, astNode))
      _astNodes.push_back(astNode);
    else
      return true;

    //--- CppNamespaceAlias ---//

    model::CppNamespaceAliasPtr nsa = std::make_shared<model::CppNamespaceAlias>();
    _namespaceAliases.push_back(nsa);

    nsa->astNodeId = astNode->id;
    nsa->entityHash = astNode->entityHash;
    nsa->name = nad_->getNameAsString();
    nsa->qualifiedName = nad_->getQualifiedNameAsString();

    return true;
  }


  bool VisitUsingDecl(clang::UsingDecl* ud_)
  {
    //--- CppAstNode ---//

    for (const clang::UsingShadowDecl* nd : ud_->shadows()) {
      model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

      astNode->astValue = getSourceText(
        _clangSrcMgr,
        ud_->getBeginLoc(),
        ud_->getLocation(),
        true);
      astNode->location = getFileLoc(ud_->getBeginLoc(), ud_->getEndLoc());
      astNode->entityHash = util::fnvHash(getUSR(nd->getTargetDecl()));

      astNode->symbolType = model::CppAstNode::SymbolType::Other;
      astNode->astType = model::CppAstNode::AstType::UsingLoc;

      astNode->id = model::createIdentifier(*astNode);

      if (insertToCache(nd, astNode))
        _astNodes.push_back(astNode);
    }

    return true;
  }

  bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* udd_)
  {
    //--- CppAstNode ---//

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    const clang::NamespaceDecl* nd = udd_->getNominatedNamespace();

    astNode->astValue = getSourceText(
      _clangSrcMgr,
      udd_->getBeginLoc(),
      udd_->getLocation(),
      true);
    astNode->location = getFileLoc(udd_->getBeginLoc(), udd_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(nd));
    astNode->symbolType = model::CppAstNode::SymbolType::Namespace;
    astNode->astType = model::CppAstNode::AstType::Usage;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(udd_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr* ce_)
  {
    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    const clang::CXXConstructorDecl* ctor = ce_->getConstructor();

    astNode->astValue = getSignature(ctor);
    astNode->location = getFileLoc(ce_->getBeginLoc(), ce_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(ctor));
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

    astNode->astValue = getSignature(functionDecl);
    astNode->location = getFileLoc(ne_->getBeginLoc(), ne_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(functionDecl));
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType = model::CppAstNode::AstType::Usage;

    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(ne_, astNode))
      _astNodes.push_back(astNode);

    _locToAstValue[ne_->getAllocatedTypeSourceInfo()->
      getTypeLoc().getBeginLoc().getRawEncoding()] = getSourceText(
        _clangSrcMgr,
        ne_->getSourceRange().getBegin(),
        ne_->getSourceRange().getEnd(),
        true);

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

    astNode->astValue = getSignature(functionDecl);
    astNode->location = getFileLoc(de_->getBeginLoc(), de_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(functionDecl));
    astNode->symbolType = model::CppAstNode::SymbolType::Function;
    astNode->astType = model::CppAstNode::AstType::Usage;
    astNode->id = model::createIdentifier(*astNode);

    if (insertToCache(de_, astNode))
      _astNodes.push_back(astNode);

    addDestructorUsage(de_->getDestroyedType(), astNode->location, de_);

    return true;
  }

  bool VisitCallExpr(clang::CallExpr* ce_)
  {
    const clang::Decl* callee = ce_->getCalleeDecl();

    if (!callee) // TODO: How can this happen?
      return true;

    const clang::NamedDecl* namedCallee
      = llvm::dyn_cast<clang::NamedDecl>(callee);

    const clang::FunctionDecl* funcCallee
      = llvm::dyn_cast<clang::FunctionDecl>(callee);

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    std::string usr = getUSR(namedCallee);

    astNode->astValue
      = funcCallee
      ? getSignature(funcCallee)
      : namedCallee->getNameAsString();
    astNode->location = getFileLoc(ce_->getBeginLoc(), ce_->getEndLoc());
    astNode->entityHash = util::fnvHash(usr);
    astNode->symbolType
      = funcCallee
      ? model::CppAstNode::SymbolType::Function
      : model::CppAstNode::SymbolType::FunctionPtr;
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

    model::CppAstNodePtr astNode;

    if (const clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(decl))
    {
      astNode = std::make_shared<model::CppAstNode>();

      if (!_contextStatementStack.empty())
      {
        clang::Stmt* context = _contextStatementStack.top();
        astNode->astValue = getSourceText(
          _clangSrcMgr,
          context->getSourceRange().getBegin(),
          context->getSourceRange().getEnd(),
          true);
      }
      else
        astNode->astValue = vd->getNameAsString();

      astNode->location = getFileLoc(dr_->getBeginLoc(), dr_->getEndLoc());
      astNode->entityHash = util::fnvHash(getUSR(vd));
      astNode->symbolType
        = isFunctionPointer(vd)
        ? model::CppAstNode::SymbolType::FunctionPtr
        : model::CppAstNode::SymbolType::Variable;
      astNode->astType
        = isWritten(dr_)
        ? model::CppAstNode::AstType::Write
        : model::CppAstNode::AstType::Read;

      astNode->id = model::createIdentifier(*astNode);
    }
    else if (const clang::EnumConstantDecl* ec
      = llvm::dyn_cast<clang::EnumConstantDecl>(decl))
    {
      astNode = std::make_shared<model::CppAstNode>();

      if (!_contextStatementStack.empty())
      {
        clang::Stmt* context = _contextStatementStack.top();
        astNode->astValue = getSourceText(
          _clangSrcMgr,
          context->getBeginLoc(),
          context->getEndLoc(),
          true);
      }
      else
        astNode->astValue = ec->getNameAsString();

      astNode->location = getFileLoc(dr_->getBeginLoc(), dr_->getEndLoc());
      astNode->entityHash = util::fnvHash(getUSR(ec));
      astNode->symbolType = model::CppAstNode::SymbolType::EnumConstant;
      astNode->astType = model::CppAstNode::AstType::Usage;

      astNode->id = model::createIdentifier(*astNode);
    }
    else
      return true;

    if (astNode && insertToCache(dr_, astNode))
      _astNodes.push_back(astNode);

    return true;
  }

  bool VisitMemberExpr(clang::MemberExpr* me_)
  {
    const clang::ValueDecl* vd = me_->getMemberDecl();
    const clang::CXXMethodDecl* method
      = llvm::dyn_cast<clang::CXXMethodDecl>(vd);

    model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

    astNode->astValue = method ? getSignature(method) : vd->getNameAsString();
    astNode->location = getFileLoc(me_->getBeginLoc(), me_->getEndLoc());
    astNode->entityHash = util::fnvHash(getUSR(vd));
    astNode->symbolType
      = method
      ? model::CppAstNode::SymbolType::Function
      : isFunctionPointer(vd)
      ? model::CppAstNode::SymbolType::FunctionPtr
      : model::CppAstNode::SymbolType::Variable;
    astNode->astType
      = isWritten(me_)
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
      rel->lhs = _entityCache.at(left->second);
      rel->rhs = _entityCache.at(right->second);
      _relations.push_back(rel);
    }

    return true;
  }

private:
  void addDestructorUsage(
    clang::QualType type_,
    const model::FileLoc& location_,
    const void* clangPtr_)
  {
    if (clang::CXXRecordDecl* rd = type_->getAsCXXRecordDecl())
    {
      if (clang::CXXDestructorDecl* dd = rd->getDestructor())
      {
        model::CppAstNodePtr astNode = std::make_shared<model::CppAstNode>();

        astNode->astValue = getSignature(dd);
        astNode->location = location_;
        astNode->entityHash = util::fnvHash(getUSR(dd));
        astNode->symbolType = model::CppAstNode::SymbolType::Function;
        astNode->astType = model::CppAstNode::AstType::Usage;
        astNode->id = model::createIdentifier(*astNode);

        if (insertToCache(clangPtr_, astNode))
          _astNodes.push_back(astNode);
      }
    }
  }

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
    return _entityCache.insert(*node_);
  }

  /**
   * This function returns a pointer to the corresponding model::File object
   * based on the given source location. The object is read from the cache of
   * the actual translation unit. If the file is not in the cache of the actual
   * translation unit yet then it will get the file from the Source Manager.
   */
  model::FilePtr getFile(const clang::SourceLocation& loc_)
  {
    std::string path = _fileLocUtil.getFilePath(loc_);

    std::unordered_map<std::string, model::FilePtr>::const_iterator it =
      _files.find(path);

    return _files[path] = it != _files.end()
      ? it->second
      : _ctx.srcMgr.getFile(path);
  }

  model::FileLoc getFileLoc(
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_)
  {
    model::FileLoc fileLoc;

    if (start_.isInvalid() || end_.isInvalid())
      fileLoc.file = getFile(start_);
    else
    {
      clang::SourceLocation realStart = start_;
      clang::SourceLocation realEnd = end_;

      if (_clangSrcMgr.isMacroBodyExpansion(start_))
        realStart = _clangSrcMgr.getExpansionLoc(start_);
      if (_clangSrcMgr.isMacroArgExpansion(start_))
        realStart = _clangSrcMgr.getSpellingLoc(start_);

      if (_clangSrcMgr.isMacroBodyExpansion(end_))
        realEnd = _clangSrcMgr.getExpansionLoc(end_);
      if (_clangSrcMgr.isMacroArgExpansion(end_))
        realEnd = _clangSrcMgr.getSpellingLoc(end_);

      if (!_isImplicit)
        _fileLocUtil.setRange(realStart, realEnd, fileLoc.range);

      fileLoc.file = getFile(realStart);
    }

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

  /**
   * This function returns true if the given type makes it possible to modify
   * (write) the value, i.e. if it is a non-const reference or pointer.
   */
  bool isWritable(const clang::QualType& type_) const
  {
    const clang::Type* typePtr = type_.getTypePtr();

    return
      ((typePtr->isReferenceType() || typePtr->isPointerType()) &&
       !typePtr->getPointeeType().isConstQualified()) ||
      typePtr->isRValueReferenceType();
  }

  /**
   * This function returns true if the given expression (which is usually a
   * DeclRefExpr) is in writing context, i.e. on the left hand side of an
   * assignment or as a function call argument passed by reference, etc.
   */
  bool isWritten(const clang::Expr* expr_) const
  {
    while (expr_)
    {
      clang::DynTypedNodeList parents
        = _astContext.getParents(*expr_);

      const clang::DynTypedNode& parent = parents[0];

      if (const clang::BinaryOperator* op = parent.get<clang::BinaryOperator>())
      {
        return
          (op->isAssignmentOp() ||
           op->isCompoundAssignmentOp() ||
           op->isShiftAssignOp()) &&
          op->getLHS() == expr_;
      }
      else if (const clang::UnaryOperator* op
        = parent.get<clang::UnaryOperator>())
      {
        return op->isIncrementDecrementOp() && op->getSubExpr() == expr_;
      }
      else if (const clang::CallExpr* call = parent.get<clang::CallExpr>())
      {
        const clang::FunctionDecl* decl = call->getDirectCallee();

        if (!decl)
          return false;

        const clang::CXXMethodDecl* method
          = llvm::dyn_cast<clang::CXXMethodDecl>(decl);

        if (method && !method->isConst() &&
            call->getNumArgs() && call->getArg(0) == expr_)
          return true;

        // The first argument of CallExpr is "this" if it's a CXXMethodDecl.
        unsigned x = method != nullptr;

        for (unsigned i = 0;
             i + x < call->getNumArgs() && i < decl->getNumParams();
             ++i)
        {
          clang::QualType paramType = decl->getParamDecl(i)->getType();
          if (isWritable(paramType) && call->getArg(i + x) == expr_)
            return true;
        }

        return false;
      }
      else if (const clang::CXXConstructExpr* call
        = parent.get<clang::CXXConstructExpr>())
      {
        const clang::CXXConstructorDecl* decl = call->getConstructor();

        if (!decl)
          return false;

        for (unsigned i = 0;
             i < call->getNumArgs() && i < decl->getNumParams();
             ++i)
        {
          clang::QualType paramType = decl->getParamDecl(i)->getType();
          if (isWritable(paramType) && call->getArg(i) == expr_)
            return true;
        }

        return false;
      }
      else if (
        parent.get<clang::CXXDeleteExpr>() ||
        parent.get<clang::ArraySubscriptExpr>())
      {
        return true;
      }

      expr_ = parent.get<clang::Expr>();
    }

    return false;
  }

  std::vector<model::CppAstNodePtr>        _astNodes;
  std::vector<model::CppEnumConstantPtr>   _enumConstants;
  std::vector<model::CppEnumPtr>           _enums;
  std::vector<model::CppFunctionPtr>       _functions;
  std::vector<model::CppRecordPtr>         _types;
  std::vector<model::CppTypedefPtr>        _typedefs;
  std::vector<model::CppVariablePtr>       _variables;
  std::vector<model::CppNamespacePtr>      _namespaces;
  std::vector<model::CppNamespaceAliasPtr> _namespaceAliases;
  std::vector<model::CppMemberTypePtr>     _members;
  std::vector<model::CppInheritancePtr>    _inheritances;
  std::vector<model::CppFriendshipPtr>     _friends;
  std::vector<model::CppRelationPtr>       _relations;

  // TODO: Maybe we don't even need a stack, if functions can't be nested.
  // Check lambda.
  // TODO: _enumStack also doesn't have to be a stack.
  std::stack<model::CppFunctionPtr> _functionStack;
  std::stack<model::CppRecordPtr>   _typeStack;
  std::stack<model::CppEnumPtr>     _enumStack;

  bool _isImplicit;
  ParserContext& _ctx;
  const clang::SourceManager& _clangSrcMgr;
  FileLocUtil _fileLocUtil;
  clang::ASTContext& _astContext;
  clang::MangleContext* _mngCtx;
  const std::string _cppSourceType;
  std::unordered_map<std::string, model::FilePtr> _files;

  EntityCache& _entityCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;

  // clang::TypeLoc for type names is like clang::DeclRefExpr for objects: it
  // represents their occurrences in the source code. Type names may occur in
  // source code in several contexts: at variable declaration, function return
  // type, inheritance, etc. By the time clang::RecursiveASTVisitor visits
  // these type locations we can't determine the enclosing context, so we can't
  // fill in their context-specific attributes such as astType and astValue.
  // These attributes are collected when the context is being visited and the
  // will be filled in CppAstNode objects in the destructor at the end.
  std::unordered_map<unsigned, model::CppAstNodePtr> _locToTypeLoc;
  std::unordered_map<unsigned, model::CppAstNode::AstType> _locToAstType;
  std::unordered_map<unsigned, std::string> _locToAstValue;

  StatementStack _stmtStack;
  // This stack has the same role as _locTo* maps. In case of
  // clang::DeclRefExpr objects we need to determine the contect of the given
  // expression. In this stack we store the deepest statement node in AST which
  // we consider to be a context. This context will serve the astValue of a
  // clang::DeclRefExpr.
  std::stack<clang::Stmt*> _contextStatementStack;
};

}
}

#endif
