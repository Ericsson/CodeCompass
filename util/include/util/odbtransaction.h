#ifndef CC_UTIL_ODBTRANSACTION_H
#define CC_UTIL_ODBTRANSACTION_H

#include <memory>
#include <future>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/tracer.hxx>
#include <odb/session.hxx>

namespace cc
{
namespace util
{

namespace internal
{
  template <typename T>
  struct Holder
  {
    template<typename F, typename ... Args>
    Holder(F func, Args&&... args)
    {
      data = func(std::forward<Args>(args)...);
    }

    T&& getValue()
    {
      return std::move(data);
    }

    T data;
  };

  template<>
  struct Holder<void>
  {
    template<typename F, typename ... Args>
    Holder(F func, Args&&... args)
    {
      func(std::forward<Args>(args)...);
    }

    void getValue()
    {
      return;
    }
  };

  class TransRestore
  {
  public:
    TransRestore() :
      _prevSession(odb::session::has_current() ?
         &odb::session::current() : nullptr),
      _prevTrans(odb::transaction::has_current() ?
         &odb::transaction::current() : nullptr)
    {
    }

    ~TransRestore()
    {
      if (_prevSession)
      {
        odb::session::current(*_prevSession);
      }

      if (_prevTrans)
      {
        odb::transaction::current(*_prevTrans);
      }
    }

  private:
    odb::session* _prevSession;
    odb::transaction* _prevTrans;
  };
}

class OdbTransaction
{
public:
  OdbTransaction(
    const std::shared_ptr<odb::database>& db_,
    bool switchCurrent_ = false)
    : _db(*db_), _switchCurrent(switchCurrent_)
  {
  }

  OdbTransaction(
    odb::database& db_,
    bool switchCurrent_ = false)
    : _db(db_), _switchCurrent(switchCurrent_)
  {
  }

  template<typename F, typename ... Args>
  auto operator()(F func, Args&&... args)
  {
    using namespace odb;

    std::unique_ptr<session> s;
    std::unique_ptr<transaction> t;
    internal::TransRestore trRestore;

    bool alreadyInTransaction = transaction::has_current();
#ifdef DATABASE_SQLITE
    // We have to disable transaction switching for SQLite (otherwise we could
    // get a deadlock).
    if (!alreadyInTransaction)
#else
    if (!alreadyInTransaction || _switchCurrent)
#endif
    {
      s = std::make_unique<session>(false);
      t = std::make_unique<transaction>(_db.begin(), false);

      session::current(*s);
      transaction::current(*t);
    }

    internal::Holder<decltype(func(std::forward<Args>(args)...))>
      holder(func, std::forward<Args>(args)...);

    if (t.get())
    {
      t->commit();
      t.reset();
      s.reset();
    }
#ifdef DATABASE_SQLITE
    // IT'S COMMENTED OUT (SLOW, VERY SLOW)
    /*else if (_switchCurrent)
    {
      // We have to commit the transaction it maybe commits some unwanted
      // changes, but we don't have better solution for SQLite (anyway SQLite
      // is only for testing).
      transaction* trans = &transaction::current();
      trans->commit();
      trans->reset(_db.begin());
    }*/
#endif

    return holder.getValue();
  }

  template<typename F, typename ... Args>
  auto runParallel(F func, Args&&... args)
  {
    return std::async(std::launch::async,
      *this, func, std::forward<Args>(args)...);
  }

private:
  odb::database& _db;
  bool _switchCurrent;
};

} // util
} // cc


#endif /* CC_UTIL_ODBTRANSACTION_H */
