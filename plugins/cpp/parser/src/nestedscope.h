#ifndef CC_PARSER_NESTEDSCOPE_H
#define CC_PARSER_NESTEDSCOPE_H

#include <clang/AST/Stmt.h>

namespace cc
{
namespace parser
{

  class NestedScope;

  class NestedStack final
  {
    friend class NestedScope;

  private:
    NestedScope* _top;

  public:
    NestedScope* Top() const { return _top; }

    NestedStack() : _top(nullptr) {}
  };

  class NestedScope
  {
  protected:
    enum class State : unsigned char
    {
      Initial,
      Expected,
      Standalone,
      Invalid,
    };

    NestedStack* _stack;
    NestedScope* _previous;
    clang::Stmt* _stmt;
    unsigned int _depth;
    State _state;

    virtual State CheckNext(clang::Stmt* stmt_) const = 0;

  public:
    NestedStack* Stack() const { return _stack; }
    NestedScope* Previous() const { return _previous; }
    clang::Stmt* Statement() const { return _stmt; }

    unsigned int PrevDepth() const
    { return _previous != nullptr ? _previous->_depth : 0; }
    unsigned int Depth() const { return _depth; }
    bool IsReal() const { return Depth() > PrevDepth(); }

    NestedScope(NestedStack* stack_, clang::Stmt* stmt_);
    virtual ~NestedScope();
  };

  class NestedTransparentScope : public NestedScope
  {
  protected:
    virtual State CheckNext(clang::Stmt* stmt_) const override;

  public:
    NestedTransparentScope(NestedStack* stack_, clang::Stmt* stmt_);
  };

  class NestedCompoundScope : public NestedTransparentScope
  {
  public:
    NestedCompoundScope(NestedStack* stack_, clang::Stmt* stmt_);
  };

  class NestedStatementScope : public NestedScope
  {
  protected:
    virtual State CheckNext(clang::Stmt* stmt_) const override;

  public:
    NestedStatementScope(NestedStack* stack_, clang::Stmt* stmt_);
  };

  class NestedOneWayScope : public NestedStatementScope
  {
  protected:
    clang::Stmt* _next;

    virtual State CheckNext(clang::Stmt* stmt_) const override;

  public:
    NestedOneWayScope(
      NestedStack* stack_,
      clang::Stmt* stmt_,
      clang::Stmt* next_
    );
  };
  
  class NestedFunctionScope : public NestedOneWayScope
  {
  public:
    NestedFunctionScope(
      NestedStack* stack_,
      clang::Stmt* next_
    );
  };
  
  class NestedTwoWayScope : public NestedStatementScope
  {
  protected:
    clang::Stmt* _next0;
    clang::Stmt* _next1;

    virtual State CheckNext(clang::Stmt* stmt_) const override;

  public:
    NestedTwoWayScope(
      NestedStack* stack_,
      clang::Stmt* stmt_,
      clang::Stmt* next0_,
      clang::Stmt* next1_
    );
  };

}
}

#endif
