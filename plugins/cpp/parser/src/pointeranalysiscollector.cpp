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
 * Determine whether constructor declaration is a smart pointer.
 * @param cd_ C++ constructor declaration.
 * @return True if the
 */
bool isSmartPointer(const std::string& name_)
{
  for (const std::string smartPtr : smartPointers)
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

  std::string name = namedCallee->getNameAsString();
  return
    std::find(allocators.begin(), allocators.end(), name)!= allocators.end();
}

/**
 * Get pointer analysis options for variables.
 */
std::set<cc::model::CppPointerAnalysis::Options> getVariableOptions(
  const clang::VarDecl* vd_)
{
  std::set<cc::model::CppPointerAnalysis::Options> options;
  const clang::Type* type = vd_->getType().getTypePtrOrNull();
  if (type && type->isReferenceType())
    options.insert(cc::model::CppPointerAnalysis::Options::Reference);

  if (type && type->isArrayType())
      options.insert(cc::model::CppPointerAnalysis::Options::Array);

  bool isParam = llvm::isa<clang::ParmVarDecl>(vd_);
  if (isParam)
    options.insert(cc::model::CppPointerAnalysis::Options::Param);

  if (isParam || vd_->isLocalVarDecl())
    options.insert(cc::model::CppPointerAnalysis::Options::StackObj);
  else
    options.insert(cc::model::CppPointerAnalysis::Options::GlobalObject);

  if (vd_->isStaticLocal() || vd_->isStaticDataMember())
    options.insert(cc::model::CppPointerAnalysis::Options::GlobalObject);

  return options;
}

std::string getSuffixFromLoc(const cc::model::FileLoc& fileLoc_)
{
  if (!fileLoc_.file)
    return std::string();

  return std::to_string(fileLoc_.file.object_id()) + ':'
       + std::to_string(fileLoc_.range.start.line) + ':'
       + std::to_string(fileLoc_.range.start.column);
}

/**
 * This function checks if the type is a pointer or a reference type.
 */
bool isPointerOrReferenceType(
  const clang::Type* type_,
  const std::string& sideType_)
{
  if (type_ && (
      type_->isAnyPointerType() ||
      type_->isReferenceType() ||
      type_->isArrayType() ||
      isSmartPointer(sideType_)))
    return true;
  return false;
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
    _fileLocUtil(astContext_.getSourceManager()),
    _cppSourceType("CPP")
{
}

cc::model::CppAstNodePtr AstNodeCreator::operator()(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_,
  bool addSuffixToMangledName_)
{
  cc::model::CppAstNodePtr astNode = std::make_shared<cc::model::CppAstNode>();
  astNode->symbolType = cc::model::CppAstNode::SymbolType::Other;
  astNode->astType = cc::model::CppAstNode::AstType::Other;
  astNode->visibleInSourceCode = false;
  astNode->astValue = astValue_;

  if (!addFileLoc(astNode, start_, end_))
    return nullptr;

  astNode->mangledName = mangledName_;

  if (addSuffixToMangledName_ && start_.isValid() && end_.isValid())
    astNode->mangledName += getSuffixFromLoc(astNode->location);

  astNode->mangledNameHash = cc::util::fnvHash(astNode->mangledName);
  astNode->id = cc::model::createIdentifier(*astNode);

  return astNode;
}

bool AstNodeCreator::addFileLoc(
  cc::model::CppAstNodePtr& astNode_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_)
{
  if (start_.isInvalid() || end_.isInvalid())
    return false;

  cc::model::FileLoc fileLoc;
  _fileLocUtil.setRange(start_, end_, fileLoc.range);
  fileLoc.file = _srcMgr.getFile(_fileLocUtil.getFilePath(start_));

  const std::string& type = fileLoc.file.load()->type;
  if (type != cc::model::File::DIRECTORY_TYPE && type != _cppSourceType)
  {
    fileLoc.file->type = _cppSourceType;
    _srcMgr.updateFile(*fileLoc.file);
  }

  astNode_->location = fileLoc;
  return true;
}

ReturnCollector::ReturnCollector(
  std::unordered_set<clang::Expr*>& collected_)
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
    _ctx(ctx_),
    _astContext(astContext_),
    _mangledNameCache(mangledNameCache_),
    _clangToAstNodeId(clangToAstNodeId_),
    _astNodes(astNodes_),
    _overridens(overridens_),
    _clangSrcMgr(astContext_.getSourceManager()),
    _astNodeCreator(ctx_.srcMgr, astContext_),
    _fileLocUtil(astContext_.getSourceManager()),
    _mngCtx(astContext_.createMangleContext()),
    _shouldCollect(true),
    _cppSourceType("CPP"),
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

  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::Literal,
    model::CppPointerAnalysis::Options::GlobalObject});

  return false;
}

bool StmtSideCollector::VisitCXXNullPtrLiteralExpr(
  clang::CXXNullPtrLiteralExpr* ne_)
{
  std::uint64_t mangledNameHash = createAstNode("nullptr", "nullptr",
    ne_->getLocStart(), ne_->getLocEnd());

  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::NullPtr});

  return false;
}

bool StmtSideCollector::VisitGNUNullExpr(clang::GNUNullExpr* ne_)
{
  std::uint64_t mangledNameHash = createAstNode("NULL", "NULL",
    ne_->getLocStart(), ne_->getLocEnd());

  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::NullPtr});

  return false;
}

bool StmtSideCollector::VisitUnaryDeref(clang::UnaryOperator* uop_)
{
  _operators += "*";
  return true;
}

bool StmtSideCollector::VisitUnaryAddrOf(clang::UnaryOperator* uop_)
{
  _operators += "&";
  return true;
}

bool StmtSideCollector::VisitVarDecl(clang::VarDecl* vd_)
{
  addStmtSide(getStmtMangledName(vd_), _operators, getVariableOptions(vd_));

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
      addStmtSide(mangledNameHash, _operators, {
        model::CppPointerAnalysis::Options::NullPtr});
      return false;
    }
  }

  std::uint64_t mangledNameHash = createAstNode(ctor->getNameAsString(),
    getMangledName(_mngCtx, ctor), ce_->getLocStart(), ce_->getLocEnd());
  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::StackObj});

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

  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::HeapObj});

  return false;
}

bool StmtSideCollector::VisitExprWithCleanups(clang::ExprWithCleanups* ec_)
{
  addStmtSide(getStmtMangledName(ec_), _operators);
  return false;
}

bool StmtSideCollector::VisitCallExpr(clang::CallExpr* ce_)
{
  clang::FunctionDecl* callee = ce_->getDirectCallee();

  // TODO: If call is a function pointer call, callee is a nullptr
  if (!callee)
  {
    return false;
  }

  //--- Check if it's a `move` operator ---//

  if (isMoveOperator(callee) && ce_->getNumArgs())
  {
    this->TraverseStmt(ce_->getArg(0));
    return false;
  }

  if (isVirtualCall(ce_))
    if (collectVirtualFunctionCalls(ce_))
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

    addStmtSide(mangledNameHash, _operators, {
      model::CppPointerAnalysis::Options::HeapObj});

    return false;
  }

  std::unordered_set<clang::Expr*> ret;
  ReturnCollector collector(ret);
  collector.collect(callee);

  ++_returnCollectorCallCount;

  if (ret.empty() || _returnCollectorCallCount > _maxReturnCount)
  {
    addStmtSide(getStmtMangledName(ce_), _operators, {
      model::CppPointerAnalysis::Options::FunctionCall});
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
  addStmtSide(getStmtMangledName(me_), _operators, {
    model::CppPointerAnalysis::Options::Member});

  return false;
}

bool StmtSideCollector::VisitDeclRefExpr(clang::DeclRefExpr* re_)
{
  clang::ValueDecl* decl = re_->getDecl();

  std::set<cc::model::CppPointerAnalysis::Options> options;
  if (const clang::VarDecl* vd = llvm::dyn_cast<clang::VarDecl>(decl))
    options = getVariableOptions(vd);

  addStmtSide(getStmtMangledName(re_), _operators, options);
  return false;
}

bool StmtSideCollector::VisitFieldDecl(clang::FieldDecl* fd_)
{
  addStmtSide(getStmtMangledName(fd_), _operators);
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

  addStmtSide(mangledNameHash, _operators, {
    model::CppPointerAnalysis::Options::InitList});

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

    for (clang::CXXMethodDecl* d : _overridens[md])
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
  const std::string operators_,
  std::set<model::CppPointerAnalysis::Options> options_)
{
  if (_isReturnType)
    options_.insert(model::CppPointerAnalysis::Options::Return);

  _collected.insert({mangledNameHash_, operators_, options_});
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

  // Some reason `src` can contain null terminated string  character(\0)
  // on which pgsql will throw an error: invalid byte sequence for
  // encoding "SQL_ASCII": 0x00. For this reason we call c_str on the string.
  return src.str().c_str();
}

std::uint64_t StmtSideCollector::createAstNode(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_,
  bool addSuffixToMangledName_)
{
  cc::model::CppAstNodePtr astNode = _astNodeCreator(astValue_, mangledName_,
    start_, end_, addSuffixToMangledName_);

  if (astNode)
  {
    if (_mangledNameCache.insert(*astNode))
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
    persistAll(_astNodes);
    persistAll(_pAnalysis);
  });
}

bool PointerAnalysisCollector::VisitCXXMethodDecl(clang::CXXMethodDecl* decl)
{
  for (auto it = decl->begin_overridden_methods();
       it != decl->end_overridden_methods();
       ++it)
  {
    clang::CXXMethodDecl* overriden = const_cast<clang::CXXMethodDecl*>(*it);
    _overridens[decl].insert(overriden);
    _overridens[overriden].insert(decl);
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
  if (isSmartPointer(ce_->getType().getAsString()) &&
      ce_->getNumArgs() > 1)
  {
    makeAssignRels(ce_->getArg(0), ce_->getArg(1));
  }

  return true;
}

void PointerAnalysisCollector::createPointerAnalysis(
  const std::set<model::CppPointerAnalysis::StmtSide>& lhs_,
  const std::set<model::CppPointerAnalysis::StmtSide>& rhs_)
{
  static std::mutex _cacheMutex;

  std::lock_guard<std::mutex> guard(_cacheMutex);
  for (const model::CppPointerAnalysis::StmtSide& lhs : lhs_)
    for (const model::CppPointerAnalysis::StmtSide& rhs : rhs_)
    {
      if (lhs.mangledNameHash && rhs.mangledNameHash)
      {
//        std::string id =
//          std::to_string(lhs.mangledNameHash) +
//          lhs.operators +
//          std::to_string(lhs.options.size()) +
//          std::to_string(rhs.mangledNameHash) +
//          rhs.operators +
//          std::to_string(rhs.options.size());

        std::string id =
          std::to_string(lhs.mangledNameHash) +
          std::to_string(rhs.mangledNameHash);

        model::CppPointerAnalysisPtr pAnalysis =
          std::make_shared<model::CppPointerAnalysis>();
        pAnalysis->lhs = lhs;
        pAnalysis->rhs = rhs;

        pAnalysis->id = util::fnvHash(id);

        if (_pointerAnalysisCache.insert(pAnalysis->id))
          _pAnalysis.push_back(std::move(pAnalysis));
      }
    }
}

std::uint64_t PointerAnalysisCollector::createAstNode(
  const std::string& astValue_,
  const std::string& mangledName_,
  const clang::SourceLocation& start_,
  const clang::SourceLocation& end_,
  bool addSuffixToMangledName_)
{
  cc::model::CppAstNodePtr astNode = _astNodeCreator(astValue_, mangledName_,
    start_, end_, addSuffixToMangledName_);

  if (astNode)
  {
    if (_mangledNameCache.insert(*astNode))
      _astNodes.push_back(astNode);

    return astNode->mangledNameHash;
  }
  return 0;
}

void PointerAnalysisCollector:: makeUndefinedRels(clang::VarDecl* lhs_)
{
  std::set<model::CppPointerAnalysis::StmtSide> lhs = collect(lhs_);
  model::CppPointerAnalysis::StmtSide rhs;
  rhs.mangledNameHash = createAstNode("undefined", "undefined",
    lhs_->getLocStart(), lhs_->getLocEnd());
  rhs.options.insert(
    model::CppPointerAnalysis::Options::Undefined);
  createPointerAnalysis(lhs, {rhs});
}

} // parser
} // cc
