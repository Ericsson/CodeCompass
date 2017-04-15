#ifndef CC_PARSER_REFCOLLECTOR_H
#define CC_PARSER_REFCOLLECTOR_H

namespace cc
{
namespace parser
{

class RefCollector : public clang::RecursiveASTVisitor<RefCollector>
{
public:
  typedef std::function<bool(const clang::Type*)> RefTester;

  RefCollector(std::unordered_set<const void*>& collected_)
    : _collected(collected_),
      _shouldCollect(true)
  {
  }

  RefCollector(std::unordered_set<const void*>& collected_,
    RefTester refTester_)
    : _collected(collected_),
      _refTester(refTester_),
      _shouldCollect(true)
  {
  }

  void collect(clang::Stmt* stmt_)
  {
    _startStmt = stmt_;
    this->TraverseStmt(_startStmt);
  }

  void collect(clang::Decl* decl_)
  {
    _startDecl = decl_;
    this->TraverseDecl(_startDecl);
  }

  bool TraverseCXXConstructExpr(clang::CXXConstructExpr* ce_)
  {
    bool didISwitchItOff = false;

    if (_startStmt != ce_ && _shouldCollect)
    {
      _shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCXXConstructExpr(ce_);

    if (didISwitchItOff)
      _shouldCollect = true;

    return true;
  }

  bool TraverseCallExpr(clang::CallExpr* ce_)
  {
    bool didISwitchItOff = false;

    if (_startStmt != ce_ && _shouldCollect)
    {
      _shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCallExpr(ce_);

    if (didISwitchItOff)
      _shouldCollect = true;

    return true;
  }

  bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* ce_)
  {
    bool didISwitchItOff = false;

    if (_startStmt != ce_ && _shouldCollect)
    {
      _shouldCollect = false;
      didISwitchItOff = true;
    }

    RecursiveASTVisitor<RefCollector>::TraverseCXXMemberCallExpr(ce_);

    if (didISwitchItOff)
      _shouldCollect = true;

    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr* re_)
  {
    tryCollectStmt(re_);
    return true;
  }

  bool VisitMemberExpr(clang::MemberExpr* me_)
  {
    tryCollectStmt(me_);
    return true;
  }

  bool VisitValueDecl(clang::ValueDecl* vd_)
  {
    tryCollectStmt(vd_);
    return true;
  }

private:
  template <typename T>
  void tryCollectStmt(T* t_)
  {
    if (!_shouldCollect)
      return;

    if (!_refTester || _refTester(t_->getType().getTypePtrOrNull()))
      _collected.insert(t_);
  }

  union
  {
    clang::Stmt* _startStmt;
    clang::Decl* _startDecl;
  };

  std::unordered_set<const void*>& _collected;
  RefTester                        _refTester;
  bool _shouldCollect = true;
};

} // parser
} // cc

#endif
