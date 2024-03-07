#ifndef CC_PARSER_NESTEDSCOPE_H
#define CC_PARSER_NESTEDSCOPE_H

#include <clang/AST/Stmt.h>

#include <util/scopedvalue.h>

namespace cc
{
namespace parser
{

  class StatementScope;

  class StatementStack final
  {
    DECLARE_SCOPED_TYPE(StatementStack)

    friend class StatementScope;

  private:
    StatementScope* _top;

  public:
    StatementScope* Top() const { return _top; }
    StatementScope* TopValid() const;

    StatementStack() : _top(nullptr) {}
  };

  class StatementScope final
  {
    DECLARE_SCOPED_TYPE(StatementScope)

    friend class StatementStack;
    
  private:
    enum class State : unsigned char
    {
      Initial,// This statement is the root in its function.
      Expected,// This statement was expected to be nested inside its parent.
      Standalone,// This statement was not expected, but not forbidden either.
      Invalid,// This statement was not expected to be on the stack.
    };

    enum class Kind : unsigned char
    {
      Unknown,// This scope has not been configured yet.
      Open,// Any statement can be nested inside this statement.
      Closed,// No other statement is allowed to be nested inside.
      OneWay,// Only one specific statement is allowed to be nested inside.
      TwoWay,// Only two specific statements are allowed to be nested inside.
    };

    StatementStack* _stack;
    StatementScope* _previous;
    clang::Stmt* _stmt;
    unsigned int _depth;
    State _state;
    Kind _kind;
    clang::Stmt* _exp0;
    clang::Stmt* _exp1;

    State CheckNext(clang::Stmt* stmt_) const;

  public:
    StatementStack* Stack() const { return _stack; }
    StatementScope* Previous() const { return _previous; }
    clang::Stmt* Statement() const { return _stmt; }

    unsigned int PrevDepth() const
    { return _previous != nullptr ? _previous->_depth : 0; }
    unsigned int Depth() const { return _depth; }
    bool IsReal() const { return Depth() > PrevDepth(); }

    StatementScope(StatementStack* stack_, clang::Stmt* stmt_);
    ~StatementScope();

    void EnsureConfigured();
    void MakeTransparent();
    void MakeCompound();
    void MakeStatement();
    void MakeOneWay(clang::Stmt* next_);
    void MakeFunction(clang::Stmt* body_);
    void MakeTwoWay(clang::Stmt* next0_, clang::Stmt* next1_);
  };

}
}

#endif
