#include "pointeranalysiscollector.h"

namespace
{

/**
 * List of smart pointers.
 */
const std::vector<std::string> smartPointers =
{
  "std::shared_ptr", "std::unique_ptr", "std::auto_ptr", "std::weak_ptr"
};

const std::vector<std::string> allocators =
{
  "malloc", "calloc", "realloc"
};

/**
 * This function returns true if the given type name contains a smart pointer
 * from smartPointers vector.
 *
 * TODO: What if the given name is a template type which contains a smart
 * pointer type among its template parameters? E.g.: MyType<std::shared_ptr>.
 * This is not a shared pointer. Can't Clang determine wheter a given type is a
 * smart pointer? Cant't the type be checked instead of its name?
 * IF we get the name of a node from Clang then probably id wouldn't contain
 * "std::" at the beginning. So strings in smartPointers vector also shouldn't
 * have "std::" as prefix.
 */
bool isSmartPointer(const std::string& name_)
{
  for (const std::string& smartPtr : smartPointers)
    if (name_.find(smartPtr) != std::string::npos)
      return true;
  return false;
}

/**
 * This function checks if the function declaration is a move operator.
 * @return True if the declaration is a move operator.
 */
bool isMoveOperator(const clang::FunctionDecl* fd_)
{
  const clang::NamedDecl* namedCallee = llvm::dyn_cast<clang::NamedDecl>(fd_);
  return namedCallee ? namedCallee->getNameAsString() == "move" : false;
}

/**
 * Return true, if the function declaration is an allocator (malloc, calloc or
 * realloc).
 */
bool isAllocatorCall(const clang::FunctionDecl* fd_)
{
  const clang::NamedDecl* namedCallee = llvm::dyn_cast<clang::NamedDecl>(fd_);

  if (!namedCallee)
    return false;

  return std::find(
    allocators.begin(),
    allocators.end(),
    namedCallee->getNameAsString()) != allocators.end();
}

/**
 * Get pointer analysis options for variables.
 */
cc::model::CppPointerAnalysis::Options_t getVariableOptions(
  const clang::VarDecl* vd_)
{
  cc::model::CppPointerAnalysis::Options_t options = 0;
  const clang::Type* type = vd_->getType().getTypePtrOrNull();

  if (type && type->isReferenceType())
    options |= cc::model::CppPointerAnalysis::Options::Reference;

  if (type && type->isArrayType())
      options |= cc::model::CppPointerAnalysis::Options::Array;

  bool isParam = llvm::isa<clang::ParmVarDecl>(vd_);
  if (isParam)
    options |= cc::model::CppPointerAnalysis::Options::Param;

  if (isParam || vd_->isLocalVarDecl())
    options |= cc::model::CppPointerAnalysis::Options::StackObj;
  else
    options |= cc::model::CppPointerAnalysis::Options::GlobalObject;

  if (vd_->isStaticLocal() || vd_->isStaticDataMember())
    options |= cc::model::CppPointerAnalysis::Options::GlobalObject;

  return options;
}

/**
 * This function checks if the type is a pointer or a reference type.
 */
bool isPointerOrReferenceType(
  const clang::Type* type_,
  const std::string& sideType_)
{
  return
    type_ && (
    type_->isAnyPointerType() ||
    type_->isReferenceType() ||
    type_->isArrayType() ||
    isSmartPointer(sideType_));
}

}

namespace cc
{
namespace parser
{

AstNodeCreator::AstNodeCreator(
  cc::parser::SourceManager& srcMgr_,
  const clang::ASTContext& astContext_)
  : _srcMgr(srcMgr_),
    _fileLocUtil(astContext_.getSourceManager())
{
}

cc::model::CppAstNodePtr AstNodeCreator::operator()(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_) const
{
  cc::model::CppAstNodePtr astNode = std::make_shared<cc::model::CppAstNode>();

  if (!setFileLoc(astNode->location, start_, end_))
    return nullptr;

  std::string suffix;
  if (astNode->location.file)
    suffix += std::to_string(astNode->location.file.object_id());
  suffix += ':' + std::to_string(astNode->location.range.start.line);
  suffix += ':' + std::to_string(astNode->location.range.start.column);

  astNode->symbolType = cc::model::CppAstNode::SymbolType::Other;
  astNode->astType = cc::model::CppAstNode::AstType::Other;
  astNode->visibleInSourceCode = false;
  astNode->astValue = astValue_;
  astNode->mangledName = mangledName_ + suffix;
  astNode->mangledNameHash = cc::util::fnvHash(astNode->mangledName);
  astNode->id = cc::model::createIdentifier(*astNode);

  return astNode;
}

bool AstNodeCreator::setFileLoc(
  cc::model::FileLoc& fileLoc_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_) const
{
  if (start_.isInvalid() || end_.isInvalid())
    return false;

  _fileLocUtil.setRange(start_, end_, fileLoc_.range);
  fileLoc_.file = _srcMgr.getFile(_fileLocUtil.getFilePath(start_));

  const std::string& type = fileLoc_.file.load()->type;
  if (type != cc::model::File::DIRECTORY_TYPE && type != "CPP")
  {
    fileLoc_.file->type = "CPP";
    _srcMgr.updateFile(*fileLoc_.file);
  }

  return true;
}

ReturnCollector::ReturnCollector(std::unordered_set<clang::Expr*>& collected_)
  : _collected(collected_)
{
}

void ReturnCollector::collect(clang::Stmt* stmt_)
{
  this->TraverseStmt(stmt_);
}

void ReturnCollector::collect(clang::Decl* decl_)
{
  this->TraverseDecl(decl_);
}

bool ReturnCollector::VisitReturnStmt(clang::ReturnStmt* rs_)
{
  clang::Expr* retValue = rs_->getRetValue();
  if (retValue/* && (
    llvm::isa<clang::CXXNullPtrLiteralExpr>(retValue) ||
    llvm::isa<clang::GNUNullExpr>(retValue) ||
    llvm::isa<clang::CXXConstructExpr>(retValue) ||
    llvm::isa<clang::CXXNewExpr>(retValue) ||
      llvm::isa<clang::DeclRefExpr>(retValue)
      )*/)
  {
    _collected.insert(retValue);
  }

  return true;
}

StmtSideCollector::StmtSideCollector(
  std::set<model::CppPointerAnalysis::StmtSide>& collected_,
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  MangledNameCache& mangledNameCache_,
  std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_,
  std::vector<model::CppAstNodePtr>& astNodes_,
  std::unordered_map<clang::CXXMethodDecl*,
      std::unordered_set<clang::CXXMethodDecl*>> overridens_)
    : _collected(collected_),
      _mangledNameCache(mangledNameCache_),
      _clangToAstNodeId(clangToAstNodeId_),
      _astNodes(astNodes_),
      _overriddens(overridens_),
      _clangSrcMgr(astContext_.getSourceManager()),
      _astNodeCreator(ctx_.srcMgr, astContext_),
      _mngCtx(astContext_.createMangleContext()),
      _operators(""),
      _isReturnType(false),
      _returnCollectorCallCount(0)
{
}

void StmtSideCollector::collect(clang::Stmt* stmt_)
{
  _startStmt = stmt_;
  this->TraverseStmt(_startStmt);
}

void StmtSideCollector::collect(clang::Decl* decl_)
{
  _startDecl = decl_;
  this->TraverseDecl(_startDecl);
}

bool StmtSideCollector::VisitStringLiteral(clang::StringLiteral* sl_)
{
  std::string value = getSourceText(sl_->getLocStart(), sl_->getLocEnd());
  std::uint64_t mangledNameHash = createAstNode(value, value,
    sl_->getLocStart(), sl_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::Literal |
    model::CppPointerAnalysis::Options::GlobalObject,
    _operators);

  return false;
}

bool StmtSideCollector::VisitCXXNullPtrLiteralExpr(
  clang::CXXNullPtrLiteralExpr* ne_)
{
  std::uint64_t mangledNameHash = createAstNode("nullptr", "nullptr",
    ne_->getLocStart(), ne_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::NullPtr,
    _operators);

  return false;
}

bool StmtSideCollector::VisitGNUNullExpr(clang::GNUNullExpr* ne_)
{
  std::uint64_t mangledNameHash = createAstNode("NULL", "NULL",
    ne_->getLocStart(), ne_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::NullPtr,
    _operators);

  return false;
}

bool StmtSideCollector::VisitUnaryDeref(clang::UnaryOperator*)
{
  _operators += "*";
  return true;
}

bool StmtSideCollector::VisitUnaryAddrOf(clang::UnaryOperator*)
{
  _operators += "&";
  return true;
}

bool StmtSideCollector::VisitVarDecl(clang::VarDecl* vd_)
{
  addStmtSide(getStmtMangledName(vd_), getVariableOptions(vd_), _operators);
  return false;
}

bool StmtSideCollector::VisitCXXConstructExpr(clang::CXXConstructExpr* ce_)
{
  const clang::CXXConstructorDecl* ctor = ce_->getConstructor();

  if (!ctor)
    return true;

  if (isSmartPointer(ce_->getType().getAsString()))
  {
    if (ce_->getNumArgs())
    {
      clang::Expr* init = ce_->getArg(0);
      this->TraverseStmt(init);
      return false;
    }
    else
    {
      std::uint64_t mangledNameHash = createAstNode("nullptr", "nullptr",
        ce_->getLocStart(), ce_->getLocEnd());
      addStmtSide(
        mangledNameHash,
        model::CppPointerAnalysis::Options::NullPtr,
        _operators);
      return false;
    }
  }

  std::uint64_t mangledNameHash = createAstNode(ctor->getNameAsString(),
    getMangledName(_mngCtx, ctor), ce_->getLocStart(), ce_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::StackObj,
    _operators);

  return false;
}

bool StmtSideCollector::VisitCXXNewExpr(clang::CXXNewExpr* ne_)
{
  const clang::FunctionDecl* newDecl = ne_->getOperatorNew();

  if (!newDecl)
    return false;

  std::string astValue = getSourceText(ne_->getLocStart(), ne_->getLocEnd());
  if (astValue.empty())
    astValue = newDecl->getNameAsString();

  std::uint64_t mangledNameHash = createAstNode(
    astValue, getMangledName(_mngCtx, newDecl),
    ne_->getLocStart(), ne_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::HeapObj,
    _operators);

  return false;
}

bool StmtSideCollector::VisitExprWithCleanups(clang::ExprWithCleanups* ec_)
{
  addStmtSide(getStmtMangledName(ec_), 0, _operators);
  return false;
}

bool StmtSideCollector::VisitCallExpr(clang::CallExpr* ce_)
{
  clang::FunctionDecl* callee = ce_->getDirectCallee();

  // TODO: If call is a function pointer call, callee is a nullptr
  if (!callee)
    return false;

  //--- Check if it's a `move` operator ---//

  if (isMoveOperator(callee) && ce_->getNumArgs())
  {
    this->TraverseStmt(ce_->getArg(0));
    return false;
  }

  if (isVirtualCall(ce_) && collectVirtualFunctionCalls(ce_))
    return false;

  if (isAllocatorCall(callee))
  {
    std::string astValue =
      getSourceText(ce_->getLocStart(), ce_->getLocEnd());

    if (astValue.empty())
      astValue = callee->getNameAsString();

    std::uint64_t mangledNameHash = createAstNode(
      astValue, getMangledName(_mngCtx, callee),
      ce_->getLocStart(), ce_->getLocEnd());

    addStmtSide(
      mangledNameHash,
      model::CppPointerAnalysis::Options::HeapObj,
      _operators);

    return false;
  }

  std::unordered_set<clang::Expr*> ret;
  ReturnCollector collector(ret);
  collector.collect(callee);

  ++_returnCollectorCallCount;

  if (ret.empty() || _returnCollectorCallCount > _maxReturnCount)
  {
    addStmtSide(
      getStmtMangledName(ce_),
      model::CppPointerAnalysis::Options::FunctionCall,
      _operators);
    return false;
  }

  _isReturnType = true;
  std::string operators = _operators;
  for (clang::Expr* expr : ret)
  {
    _operators = operators;
    this->TraverseStmt(expr);
  }

  return false;
}

bool StmtSideCollector::VisitMemberExpr(clang::MemberExpr* me_)
{
  addStmtSide(
    getStmtMangledName(me_),
    model::CppPointerAnalysis::Options::Member,
    _operators);

  return false;
}

bool StmtSideCollector::VisitDeclRefExpr(clang::DeclRefExpr* re_)
{
  clang::ValueDecl* decl = re_->getDecl();

  model::CppPointerAnalysis::Options_t options = 0;
  if (const clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(decl))
    options = getVariableOptions(vd);

  addStmtSide(getStmtMangledName(re_), options, _operators);
  return false;
}

bool StmtSideCollector::VisitFieldDecl(clang::FieldDecl* fd_)
{
  addStmtSide(getStmtMangledName(fd_), 0, _operators);
  return false;
}

bool StmtSideCollector::VisitInitListExpr(clang::InitListExpr* il_)
{
  std::string astValue = getSourceText(il_->getLocStart(), il_->getLocEnd());
  if (astValue.empty())
    astValue = "Init list";

  std::string mangledName = std::to_string(util::fnvHash(astValue));
  std::uint64_t mangledNameHash = createAstNode(
    astValue, mangledName,
    il_->getLocStart(), il_->getLocEnd());

  addStmtSide(
    mangledNameHash,
    model::CppPointerAnalysis::Options::InitList,
    _operators);

  return false;
}

bool StmtSideCollector::VisitConditionalOperator(
  clang::ConditionalOperator* co_)
{
  clang::Expr* lhs = co_->getLHS();
  clang::Expr* rhs = co_->getRHS();

  if (clang::ConditionalOperator* co =
    llvm::dyn_cast<clang::ConditionalOperator>(lhs))
  {
    VisitConditionalOperator(co);
  }

  if (clang::ConditionalOperator* co =
    llvm::dyn_cast<clang::ConditionalOperator>(rhs))
  {
    VisitConditionalOperator(co);
  }

  this->TraverseStmt(lhs);
  this->TraverseStmt(rhs);

  return false;
}

bool StmtSideCollector::collectVirtualFunctionCalls(const clang::CallExpr* ce_)
{
  const clang::CXXMemberCallExpr* memberCallExpr
    = llvm::dyn_cast<clang::CXXMemberCallExpr>(ce_);

  if (!memberCallExpr)
    return false;

  //--- If the method is not virtual then return false ---//

  clang::CXXMethodDecl* method = memberCallExpr->getMethodDecl();

  std::unordered_set<clang::Expr*> ret;
  ReturnCollector collector(ret);
  std::queue<clang::CXXMethodDecl*> q;
  std::unordered_set<clang::CXXMethodDecl*> processed;
  q.push(method);

  while (!q.empty())
  {
    clang::CXXMethodDecl* md = q.front();
    processed.insert(md);

    for (clang::CXXMethodDecl* d : _overriddens[md])
    {
      collector.collect(d);
      if (processed.find(d) == processed.end())
        q.push(d);
      ++_returnCollectorCallCount;
    }

    if (ret.empty() || _returnCollectorCallCount > _maxReturnCount)
      break;

    q.pop();
  }

  if (ret.empty())
    return false;

  _isReturnType = true;
  std::string operators = _operators;
  for (clang::Expr* expr : ret)
  {
    _operators = operators;
    this->TraverseStmt(expr);
  }

  return true;
}

bool StmtSideCollector::isVirtualCall(const clang::CallExpr* ce_) const
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

void StmtSideCollector::addStmtSide(
  std::uint64_t mangledNameHash_,
  model::CppPointerAnalysis::Options_t options_,
  const std::string& operators_)
{
  if (_isReturnType)
    options_ |= model::CppPointerAnalysis::Options::Return;

  _collected.insert({mangledNameHash_, options_, operators_});
}

std::string StmtSideCollector::getSourceText(
  const clang::SourceLocation& begin_,
  const clang::SourceLocation& end_)
{
  clang::CharSourceRange range = clang::CharSourceRange::getTokenRange(
  _clangSrcMgr.getSpellingLoc(begin_), _clangSrcMgr.getSpellingLoc(end_));

  if (range.isInvalid())
    return std::string();

  clang::LangOptions langOpts;
  clang::StringRef src =
    clang::Lexer::getSourceText(range, _clangSrcMgr, langOpts);

  // For some reason `src` can contain null terminated string character(\0)
  // on which pgsql will throw an error: invalid byte sequence for
  // encoding "SQL_ASCII": 0x00. For this reason we call c_str on the string.
  return src.str().c_str();
}

std::uint64_t StmtSideCollector::createAstNode(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_)
{
  cc::model::CppAstNodePtr astNode = _astNodeCreator(astValue_, mangledName_,
    start_, end_);

  if (astNode)
  {
    if (_mangledNameCache.insert(
          std::make_pair(astNode->id, astNode->mangledNameHash)))
      _astNodes.push_back(astNode);

    return astNode->mangledNameHash;
  }
  return 0;
}

std::unordered_set<std::uint64_t> PointerAnalysisCollector::_sIdCache;

PointerAnalysisCollector::PointerAnalysisCollector(
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  MangledNameCache& mangledNameCache_,
  IdCache& pointerAnalysisCache_,
  std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
  : _ctx(ctx_),
    _astContext(astContext_),
    _mangledNameCache(mangledNameCache_),
    _pointerAnalysisCache(pointerAnalysisCache_),
    _clangToAstNodeId(clangToAstNodeId_),
    _astNodeCreator(ctx_.srcMgr, astContext_)
{
}

PointerAnalysisCollector::~PointerAnalysisCollector()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    util::persistAll(_astNodes, _ctx.db);
    util::persistAll(_pAnalysis, _ctx.db);
  });
}

bool PointerAnalysisCollector::VisitCXXMethodDecl(clang::CXXMethodDecl* decl)
{
  for (auto it = decl->begin_overridden_methods();
       it != decl->end_overridden_methods();
       ++it)
  {
    clang::CXXMethodDecl* overriden = const_cast<clang::CXXMethodDecl*>(*it);
    _overriddens[decl].insert(overriden);
    _overriddens[overriden].insert(decl);
  }

  return true;
}

bool PointerAnalysisCollector::VisitBinaryOperator(clang::BinaryOperator* bop_)
{
  if (bop_->isAssignmentOp() ||
      bop_->isCompoundAssignmentOp() ||
      bop_->isShiftAssignOp())
  {
    clang::Expr* lhs = bop_->getLHS();
    const clang::Type* type = lhs->getType().getTypePtrOrNull();
    if (!isPointerOrReferenceType(type, lhs->getType().getAsString()))
      return true;

    makeAssignRels(bop_->getLHS(), bop_->getRHS());
  }

  return true;
}

bool PointerAnalysisCollector::VisitVarDecl(clang::VarDecl* vd_)
{
  const clang::Type* type = vd_->getType().getTypePtrOrNull();
  if (!isPointerOrReferenceType(type, vd_->getType().getAsString()))
    return true;

  clang::Expr* init = vd_->getInit();
  if (!init && !llvm::isa<clang::ParmVarDecl>(vd_))
    makeUndefinedRels(vd_);
  else
    makeAssignRels(vd_, init);

  return true;
}

bool PointerAnalysisCollector::VisitCallExpr(clang::CallExpr* ce_)
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

bool PointerAnalysisCollector::VisitCXXConstructorDecl(
  clang::CXXConstructorDecl* cd_)
{
  for (auto it = cd_->init_begin(); it != cd_->init_end(); ++it)
  {
    clang::CXXCtorInitializer* init = *it;
    clang::FieldDecl* member = init->getMember();

    if (!member || init->getSourceOrder() == -1)
      continue;

    makeAssignRels(member, init->getInit());
  }

  return true;
}

bool PointerAnalysisCollector::VisitCXXOperatorCallExpr(
  clang::CXXOperatorCallExpr* ce_)
{
  if (isSmartPointer(ce_->getType().getAsString()) && ce_->getNumArgs() > 1)
    makeAssignRels(ce_->getArg(0), ce_->getArg(1));

  return true;
}

void PointerAnalysisCollector::createPointerAnalysis(
  const std::set<model::CppPointerAnalysis::StmtSide>& lhs_,
  const std::set<model::CppPointerAnalysis::StmtSide>& rhs_)
{
  for (const model::CppPointerAnalysis::StmtSide& lhs : lhs_)
    for (const model::CppPointerAnalysis::StmtSide& rhs : rhs_)
      if (lhs.mangledNameHash && rhs.mangledNameHash)
      {
        model::CppPointerAnalysisPtr pAnalysis =
          std::make_shared<model::CppPointerAnalysis>();

        pAnalysis->lhs = lhs;
        pAnalysis->rhs = rhs;
        pAnalysis->id = model::createIdentifier(*pAnalysis);

        if (_pointerAnalysisCache.insert(pAnalysis->id))
          _pAnalysis.push_back(pAnalysis);
      }
}

std::uint64_t PointerAnalysisCollector::createAstNode(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_)
{
  cc::model::CppAstNodePtr astNode = _astNodeCreator(astValue_, mangledName_,
    start_, end_);

  if (astNode)
  {
    if (_mangledNameCache.insert(
          std::make_pair(astNode->id, astNode->mangledNameHash)))
      _astNodes.push_back(astNode);

    return astNode->mangledNameHash;
  }
  return 0;
}

void PointerAnalysisCollector::makeUndefinedRels(clang::VarDecl* lhs_)
{
  std::set<model::CppPointerAnalysis::StmtSide> lhs = collect(lhs_);
  model::CppPointerAnalysis::StmtSide rhs;
  rhs.mangledNameHash = createAstNode("undefined", "undefined",
    lhs_->getLocStart(), lhs_->getLocEnd());
  rhs.options |= model::CppPointerAnalysis::Options::Undefined;
  createPointerAnalysis(lhs, {rhs});
}

} // parser
} // cc
