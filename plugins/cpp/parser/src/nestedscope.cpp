#include "nestedscope.h"

namespace cc
{
namespace parser
{

  StatementScope* StatementStack::TopValid() const
  {
    StatementScope* top = _top;
    while (top != nullptr && top->_state == StatementScope::State::Invalid)
      top = top->Previous();
    return top;
  }


  StatementScope::State StatementScope::CheckNext(clang::Stmt* stmt_)
  {
    if (_state == State::Invalid)
      return State::Invalid;
    EnsureConfigured();
    switch (_kind)
    {
      case Kind::Open:
        return State::Standalone;
      case Kind::Closed:
        return State::Invalid;
      case Kind::OneWay:
        return (stmt_ == _exp0)
          ? State::Expected : State::Invalid;
      case Kind::TwoWay:
        return (stmt_ == _exp0 || stmt_ == _exp1)
          ? State::Expected : State::Invalid;
      default:
        assert(false && "This scope has not been configured yet.");
        return State::Invalid;
    }
  }


  StatementScope::StatementScope(
    StatementStack* stack_,
    clang::Stmt* stmt_
  ) :
    _stack(stack_),
    _previous(_stack->_top),
    _stmt(stmt_),
    _depth(0),
    _state(State::Initial),
    _kind(Kind::Unknown),
    _exp0(nullptr),
    _exp1(nullptr)
  {
    if (_previous != nullptr)
    {
      _depth = _previous->_depth;
      if (_stmt != nullptr)
        _state = _previous->CheckNext(_stmt);
    }
    _stack->_top = this;
  }

  StatementScope::~StatementScope()
  {
    assert(_stack->_top == this &&
      "Scope destruction order has been violated.");
    _stack->_top = _previous;
  }


  void StatementScope::EnsureConfigured()
  {
    if (_kind == Kind::Unknown)
      MakeStatement();
  }

  void StatementScope::MakeTransparent()
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // Anything inside a transparent statement block is standalone,
    // we do not expect any particular type of statement to be nested in it.
    _kind = Kind::Open;
  }

  void StatementScope::MakeCompound()
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // Anything inside a compound statement block is standalone,
    // we do not expect any particular type of statement to be nested in it.
    _kind = Kind::Open;

    // A compound statement block only counts as a non-transparent scope
    // when it is not the expected body of any other statement.
    if (_state == State::Standalone)
      ++_depth;
  }

  void StatementScope::MakeStatement()
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // A non-specialized statement scope is a single statement.
    // Anything inside a single statement (e.g sub-expressions) is not
    // something to be counted towards the total bumpiness of a function.
    _kind = Kind::Closed;

    // As long as the statement itself is valid on the stack,
    // it is a real scoped statement.
    if (_state != State::Invalid)
      ++_depth;
  }

  void StatementScope::MakeOneWay(clang::Stmt* next_)
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // A one-way scope expects a particular statement to be nested inside it.
    // Anything else above it on the stack is invalid.
    _kind = Kind::OneWay;
    _exp0 = next_;

    // As long as the statement itself is valid on the stack,
    // it is a real scoped statement.
    if (_state != State::Invalid)
      ++_depth;
  }

  void StatementScope::MakeFunction(clang::Stmt* body_)
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // A function scope expects its body to be nested inside it.
    // Anything else above it on the stack is invalid.
    _kind = Kind::OneWay;
    _exp0 = body_;

    // The level of nestedness always starts from zero at the function level.
    _depth = 0;
  }

  void StatementScope::MakeTwoWay(clang::Stmt* next0_, clang::Stmt* next1_)
  {
    assert(_kind == Kind::Unknown &&
      "Scope has already been configured; it cannot be reconfigured.");

    // A two-way scope expects either of two particular statements to be
    // nested inside it. Anything else above it on the stack is invalid.
    _kind = Kind::TwoWay;
    _exp0 = next0_;
    _exp1 = next1_;

    // As long as the statement itself is valid on the stack,
    // it is a real scoped statement.
    if (_state != State::Invalid)
      ++_depth;
  }

}
}
