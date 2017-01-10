#ifndef CC_PARSER_POINTERANALYSISCOLLECTOR_H
#define CC_PARSER_POINTERANALYSISCOLLECTOR_H

#include <vector>
#include <queue>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ASTContext.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cpppointeranalysis.h>
#include <model/cpppointeranalysis-odb.hxx>

#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>

#include <util/odbtransaction.h>
#include <util/logutil.h>

#include "symbolhelper.h"
#include "manglednamecache.h"
#include "idcache.h"
#include "filelocutil.h"

namespace cc
{
namespace parser
{

/**
 * This class is a helper class to create AST nodes.
 */
class AstNodeCreator
{
public:
  AstNodeCreator(
    cc::parser::SourceManager& srcMgr_,
    const clang::ASTContext& astContext_);

  cc::model::CppAstNodePtr operator()(
    const std::string& astValue_,
    const std::string& mangledName_,
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_,
    bool addSuffixToMangledName_ = true);

private:

  /**
   * Set AST node file location.
   */
  bool addFileLoc(
    cc::model::CppAstNodePtr& astNode_,
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_);

  cc::parser::SourceManager& _srcMgr;
  cc::parser::FileLocUtil _fileLocUtil;
  const std::string _cppSourceType;
};

/**
 * This class is a helper class to collect return statements from a clang Decl
 * or Stmt.
 */
class ReturnCollector : public clang::RecursiveASTVisitor<ReturnCollector>
{
public:
  ReturnCollector(std::unordered_set<clang::Expr*>& collected_);

  void collect(clang::Stmt* stmt_);

  void collect(clang::Decl* decl_);

  bool VisitReturnStmt(clang::ReturnStmt* rs_);

private:
  std::unordered_set<clang::Expr*>& _collected;
};

/**
 * This class helps to process statement side.
 */
class StmtSideCollector : public clang::RecursiveASTVisitor<StmtSideCollector>
{
public:
  StmtSideCollector(
    std::set<model::CppPointerAnalysis::StmtSide>& collected_,
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_,
    std::vector<model::CppAstNodePtr>& astNodes_,
    std::unordered_map<clang::CXXMethodDecl*,
      std::unordered_set<clang::CXXMethodDecl*>> overridens_);

  void collect(clang::Stmt* stmt_);

  void collect(clang::Decl* decl_);

  /**
   * This function visits string literals and creates an AST node for it with
   * a unique id.
   */
  bool VisitStringLiteral(clang::StringLiteral* sl_);

  /**
   * This function visits the `nullptr` literal and creates an AST node for it
   * with a unique id.
   */
  bool VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr* ne_);

  /**
   * This function visits the `NULL` literal and creates an AST node for it
   * with a unique id.
   */
  bool VisitGNUNullExpr(clang::GNUNullExpr* ne_);

  /**
   * Traverse dereference (`*`) operator
   */
  bool VisitUnaryDeref(clang::UnaryOperator* uop_);

  /**
   * Traverse address of (`&`) operator.
   */
  bool VisitUnaryAddrOf(clang::UnaryOperator* uop_);

  /**
   * Visit variable declaration. If the variable is a reference type, adds an
   * address of operator (`&`) to _operators.
   */
  bool VisitVarDecl(clang::VarDecl* vd_);

  /**
   * This function visits C++ constructor expressions.
   * If the constructor call is a smart pointer it will visit the smart pointer
   * argument or create a nullptr if no argument is specified, otherwise it will
   * create an AST node for the constructor call.
   */
  bool VisitCXXConstructExpr(clang::CXXConstructExpr* ce_);

  /**
   * This function visits c++ new operator and creates an AST node for it whith
   * a unique id. The AST node value will be set from the source text.
   */
  bool VisitCXXNewExpr(clang::CXXNewExpr* ne_);

  bool VisitExprWithCleanups(clang::ExprWithCleanups* ec_);

  /**
   * This function visits a function call expression.
   * There are multiple cases:
   *  - Call is a `move operator`: traverse the argument.
   *  - Call is a virtual call: collects virtual function return statements.
   *  - Otherwise try to collect call expression return statements and visits
   *    them.
   */
  bool VisitCallExpr(clang::CallExpr* ce_);

  /**
   * This function visits a member expression.
   */
  bool VisitMemberExpr(clang::MemberExpr* me_);

  /**
   * This function visits a reference to a declared variable, function,
   * enum, etc.
   */
  bool VisitDeclRefExpr(clang::DeclRefExpr* re_);

  /**
   * This function visits a member of a struct/union/class.
   */
  bool VisitFieldDecl(clang::FieldDecl* fd_);

  /**
   * This function will visit an initializer list, which can be used to
   * initialize objects of different types, including struct/class/union types,
   * arrays, and vectors. For example:
   *
   * @code
   * int* v[] = {new int(0), new int(1)};
   * @code
   */
  bool VisitInitListExpr(clang::InitListExpr* il_);

  /**
   * This function will visit the ?: ternary operator.
   *
   * @brief It will be recursively called if any side of the operator is also a
   * conditional operator. For example:
   *
   * @code
   * int* x = b1
   *   ? new int(0
   *   : b2
   *     ? new int(1)
   *     : new int(2);
   * @code
   */
  bool VisitConditionalOperator(clang::ConditionalOperator* co_);

private:

  /**
   * Collect virtual function return statements and visits them.
   */
  bool collectVirtualFunctionCalls(const clang::CallExpr* ce_);

  /**
   * Return true if a function call expression is a virtual call.
   */
  bool isVirtualCall(const clang::CallExpr* ce_) const;

  /**
   * Add statement side to the collection.
   */
  void addStmtSide(
    std::uint64_t mangledNameHash_,
    const std::string operators_,
    std::set<model::CppPointerAnalysis::Options> options_ = {});

  /**
   * Returns a string for the source that the range encompasses.
   */
  std::string getSourceText(
    const clang::SourceLocation& begin_,
    const clang::SourceLocation& end_);

  /**
   * Creates an AST node an insert it to the collection if it doesn't exist
   * already.
   */
  std::uint64_t createAstNode(
    const std::string& astValue_,
    const std::string& mangledName_,
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_,
    bool addSuffixToMangledName_ = true);

  template <typename T>
  std::uint64_t getStmtMangledName(T* t_)
  {
    auto it = _clangToAstNodeId.find(t_);
    if (it != _clangToAstNodeId.end())
       if (_mangledNameCache.contain(it->second))
        return _mangledNameCache.at(it->second);

    return 0;
  }

  union
  {
    clang::Stmt* _startStmt;
    clang::Decl* _startDecl;
  };

  const int _maxReturnCount = 5;

  std::set<model::CppPointerAnalysis::StmtSide>& _collected;
  ParserContext& _ctx;
  clang::ASTContext& _astContext;
  MangledNameCache& _mangledNameCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;
  std::vector<model::CppAstNodePtr>& _astNodes;
  std::unordered_map<clang::CXXMethodDecl*,
    std::unordered_set<clang::CXXMethodDecl*>> _overridens;
  const clang::SourceManager& _clangSrcMgr;
  AstNodeCreator _astNodeCreator;
  FileLocUtil _fileLocUtil;
  clang::MangleContext* _mngCtx;
  bool _shouldCollect;
  const std::string _cppSourceType;
  std::string _operators;
  bool _isReturnType;
  int _returnCollectorCallCount;
};

/**
 * This class helps to collect pointer analysis statements.
 */
class PointerAnalysisCollector
  : public clang::RecursiveASTVisitor<PointerAnalysisCollector>
{
public:
  PointerAnalysisCollector(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    IdCache& pointerAnalysisCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_);

  ~PointerAnalysisCollector();

  /**
   * Collect overridden functions for method declarations.
   */
  bool VisitCXXMethodDecl(clang::CXXMethodDecl* decl);

  /**
   * Visits the binary assign binary operator.
   *
   * @code
   * int* x;
   * x = new int(0); // Collects this statement.
   * @code
   */
  bool VisitBinaryOperator(clang::BinaryOperator* bop_);

  /**
   * Visit variable declarations.
   * E.g. (1.) T* x1; (2.) T* x2 = new T(); (3.) T* x3 = x1;
   * @note: For function parameter which are not being initialized getInit()
   * function will returns a null pointer.
   */
  bool VisitVarDecl(clang::VarDecl* vd_);

  /**
   * Collect statement for function call parameters.
   *
   * @code
   * void f(int& param_) {}
   * f(y); // Collects param = y statement.
   * @code
   */
  bool VisitCallExpr(clang::CallExpr* ce_);

  /**
   * Creates statements for constructor initializers.
   */
  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl* cd_);

  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* ce_);

private:

  void createPointerAnalysis(
    const std::set<model::CppPointerAnalysis::StmtSide>& lhs_,
    const std::set<model::CppPointerAnalysis::StmtSide>& rhs_);

  std::uint64_t createAstNode(
    const std::string& astValue_,
    const std::string& mangledName_,
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_,
    bool addSuffixToMangledName_ = true);

  void makeUndefinedRels(clang::VarDecl* lhs_);

  template <typename T>
  std::set<model::CppPointerAnalysis::StmtSide> collect(T* s_)
  {
    std::set<model::CppPointerAnalysis::StmtSide> ret;

    StmtSideCollector rc(ret, _ctx, _astContext, _mangledNameCache,
      _clangToAstNodeId, _astNodes, _overridens);

    rc.collect(s_);

    return ret;
  }

  template <typename T1, typename T2>
  void makeAssignRels(T1* leftSide_, T2* rightSide_)
  {
    std::set<model::CppPointerAnalysis::StmtSide> lhs = collect(leftSide_);
    std::set<model::CppPointerAnalysis::StmtSide> rhs = collect(rightSide_);

    createPointerAnalysis(lhs, rhs);
  }

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

  std::vector<model::CppAstNodePtr>         _astNodes;
  std::vector<model::CppPointerAnalysisPtr> _pAnalysis;

  std::unordered_map<clang::CXXMethodDecl*,
    std::unordered_set<clang::CXXMethodDecl*>> _overridens;
  static std::unordered_set<std::uint64_t> _sIdCache;
  ParserContext& _ctx;
  clang::ASTContext& _astContext;
  MangledNameCache& _mangledNameCache;
  IdCache& _pointerAnalysisCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;
  AstNodeCreator _astNodeCreator;
};

} // parser
} // cc

#endif // CC_PARSER_POINTERANALYSISCOLLECTOR_H
