#include "nestedscope.h"

namespace cc
{
namespace parser
{

  NestedScope::NestedScope(
    NestedStack* stack_,
    clang::Stmt* stmt_
  ) :
    _stack(stack_),
    _previous(_stack->_top),
    _stmt(stmt_),
    _depth(0),
    _state(State::Initial)
  {
    if (_previous != nullptr)
    {
      _depth = _previous->_depth;
      if (_stmt != nullptr)
        _state = _previous->CheckNext(_stmt);
    }

    if (_state != State::Invalid)
      _stack->_top = this;
  }

  NestedScope::~NestedScope()
  {
    if (_state != State::Invalid)
    {
      assert(_stack->_top == this &&
        "Scope destruction order has been violated.");
      _stack->_top = _previous;
    }
  }


  NestedScope::State NestedTransparentScope::CheckNext(clang::Stmt*) const
  {
    // Anything inside a transparent statement block is standalone,
    // we do not expect any particular type of statement to be nested in it.
    return State::Standalone;
  }

  NestedTransparentScope::NestedTransparentScope(
    NestedStack* stack_,
    clang::Stmt* stmt_
  ) :
    NestedScope(stack_, stmt_)
  {}


  NestedCompoundScope::NestedCompoundScope(
    NestedStack* stack_,
    clang::Stmt* stmt_
  ) :
    NestedTransparentScope(stack_, stmt_)
  {
    // A compound statement block only counts as a non-transparent scope
    // when it is not the expected body of any other statement.
    if (_state == State::Standalone)
      ++_depth;
  }


  NestedScope::State NestedStatementScope::CheckNext(clang::Stmt*) const
  {
    // A non-specialized statement scope is a single statement.
    // Anything inside a single statement (e.g sub-expressions) is not
    // something to be counted towards the total bumpiness of a function.
    return State::Invalid;
  }

  NestedStatementScope::NestedStatementScope(
    NestedStack* stack_,
    clang::Stmt* stmt_
  ) :
    NestedScope(stack_, stmt_)
  {
    // As long as the statement itself is valid on the stack,
    // it is a real scoped statement.
    if (_state != State::Invalid)
      ++_depth;
  }


  NestedScope::State NestedOneWayScope::CheckNext(clang::Stmt* stmt_) const
  {
    // A one-way scope expects a particular statement to be nested inside it.
    // Anything else above it on the stack is invalid.
    return (stmt_ == _next)
      ? State::Expected : State::Invalid;
  }

  NestedOneWayScope::NestedOneWayScope(
    NestedStack* stack_,
    clang::Stmt* stmt_,
    clang::Stmt* next_
  ) :
    NestedStatementScope(stack_, stmt_),
    _next(next_)
  {}


  NestedFunctionScope::NestedFunctionScope(
    NestedStack* stack_,
    clang::Stmt* next_
  ) :
    NestedOneWayScope(stack_, nullptr, next_)
  {
    // The level of nestedness always starts from zero at the function level.
    _depth = 0;
  }


  NestedScope::State NestedTwoWayScope::CheckNext(clang::Stmt* stmt_) const
  {
    // A two-way scope expects either of two particular statements to be
    // nested inside it. Anything else above it on the stack is invalid.
    return (stmt_ == _next0 || stmt_ == _next1)
      ? State::Expected : State::Invalid;
  }

  NestedTwoWayScope::NestedTwoWayScope(
    NestedStack* stack_,
    clang::Stmt* stmt_,
    clang::Stmt* next0_,
    clang::Stmt* next1_
  ) :
    NestedStatementScope(stack_, stmt_),
    _next0(next0_),
    _next1(next1_)
  {}

}
}
