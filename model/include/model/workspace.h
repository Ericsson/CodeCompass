#ifndef BASE_WORKSPACE_H
#define BASE_WORKSPACE_H

#include <memory>
#include <unistd.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/connection.hxx>
#include <odb/schema-catalog.hxx>

#ifdef DATABASE_SQLITE
#  include <odb/sqlite/database.hxx>
#endif

#ifdef DATABASE_PGSQL
#  include <odb/pgsql/database.hxx>
#endif

#include <util/streamlog.h>
#include <util/odbtransaction.h>
#include <util/util.h>

#include <model/statistics.h>
#include <model/statistics-odb.hxx>

namespace cc
{
namespace model
{

typedef std::unique_ptr<odb::transaction> transaction;

class Workspace
{
public:
  enum OpenMode
  {
    Normal,
    Create
  };

public:
  static std::shared_ptr<Workspace> getCreateWorkspace(
    std::string database_, OpenMode openMode_ = Normal)
  {
    std::shared_ptr<Workspace> ws(new Workspace());

#ifdef DATABASE_SQLITE
    if (openMode_ == Create)
    {
      if (database_.back() != ';')
      {
        database_ += ';';
      }
      database_ += "create";
    }
#endif // DATABASE_SQLITE

    ws->_db = util::createDatabase(database_);
    if (!ws->_db)
    {
      SLog(util::CRITICAL) <<
        "Failed to open database! Check your connection string!";

      throw std::runtime_error("Wrong connection string");
    }

#ifdef DATABASE_SQLITE
    odb::connection_ptr c(ws->_db->connection());
    c->execute("PRAGMA foreign_keys=OFF");

    try
    {
      odb::transaction t(c->begin());
        odb::schema_catalog::create_schema(*(ws->_db), "",
          openMode_ == Normal ? false : true);
      t.commit();
    }
    catch (const odb::exception&)
    {
      // If we continue the parsing process then the tables already exists.
    }

    c->execute("PRAGMA foreign_keys=ON");
#endif // DATABASE_SQLITE

    return ws;
  }

  transaction getTransaction()
  {
    return transaction(new odb::transaction(_db->begin()));
  }

  odb::database* getDb()
  {
    return _db.get();
  }

  template<typename T>
  void persist(T& t)
  {
    _db->persist(t);
  }

  void addStatistics(const std::string& group,
    const std::string& key, int val)
  {
    util::OdbTransaction t(std::shared_ptr<odb::database>(
      getDb(), util::NoDelete()));

    t([&, this]()
      {
        Statistics stat;

        stat.group = group;
        stat.key   = key;
        stat.value = val;

        persist(stat);
      });
  }

#ifdef DATABASE_SQLITE
  void close()
  {
	  _db.reset();
  }

  void reset(const std::string& database_)
  {
    _db = util::createDatabase(database_);
  }
#endif // DATABASE_SQLITE

private:
  Workspace() {}
  Workspace(const Workspace&) = delete;
  Workspace& operator=(const Workspace&) = delete;

  std::shared_ptr<odb::database> _db;
};

using WorkspacePtr = std::shared_ptr<Workspace>;

} // model
} // cc

#endif
