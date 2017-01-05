#include "database.h"

#include <tuple>

#include <boost/algorithm/string.hpp>

#ifdef DATABASE_MYSQL
#include <odb/mysql/database.hxx>
#endif
#ifdef DATABASE_SQLITE
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/connection-factory.hxx>
#endif
#ifdef DATABASE_PGSQL
#include <odb/pgsql/database.hxx>
#endif

namespace
{

/**
 * Helper function for creating an ODB options vector.
 *
 * @param optStr_ a user connection string without the db type
 * @return a vector of options.
 */
std::vector<std::string> createOptionsVector(const std::string& optStr_)
{
  std::vector<std::string> connOpts;
  boost::split(connOpts, optStr_, boost::is_any_of(";"));

  std::vector<std::string> opts = { " " }; // for some reason ODB needs it
  for (std::string opt : connOpts)
  {
    if (opt.empty())
    {
      continue;
    }

    std::string val;
    std::size_t assignment = opt.find('=');
    if (assignment != std::string::npos)
    {
      val = opt.substr(assignment + 1);
      opt = opt.substr(0, assignment);
    }

#ifdef DATABASE_PGSQL
    if (opt == "passwdfile")
    {
      ::setenv("PGPASSFILE", val.c_str(), 1);
      continue;
    }
#endif

    opts.emplace_back("--" + opt);
    if (!val.empty())
    {
      opts.emplace_back(std::move(val));
    }
  }

  return opts;
}

} // anonymous namespace

namespace cc
{
namespace service
{
namespace stat
{

std::shared_ptr<odb::database> openDatabase(const std::string& conn_)
{
  std::shared_ptr<odb::database> db;
  std::size_t colon = conn_.find(':');
  if (colon == std::string::npos)
  {
    return db;
  }

  std::string database = conn_.substr(0, colon);
  auto opts = createOptionsVector(conn_.substr(colon + 1));

  std::vector<const char*> odbOpts(opts.size());
  for (std::size_t i = 0; i < opts.size(); ++i)
  {
    odbOpts[i] = opts[i].c_str();
  }

  int odbArgc = static_cast<int>(odbOpts.size());
  char** odbArgv = const_cast<char**>(odbOpts.data());

#ifdef DATABASE_MYSQL
  if (database == "mysql")
  {
    db.reset(new odb::mysql::database(odbArgc, odbArgv));
  }
#endif
#ifdef DATABASE_SQLITE
  if (database == "sqlite")
  {
    std::unique_ptr<odb::sqlite::single_connection_factory> connFact(
      new odb::sqlite::single_connection_factory());
    db.reset(new odb::sqlite::database(odbArgc, odbArgv,
      false,                                      // Erase flag
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, // SQLite db flags
      true,                                       // Foreign keys
      "",                                         // VFS
      std::move(connFact)));
  }
#endif
#ifdef DATABASE_PGSQL
  if (database == "pgsql")
  {
    db.reset(new odb::pgsql::database(odbArgc, odbArgv));
  }
#endif
  
  // Test database connection
  try
  {
    if (!db)
    {
      return nullptr;
    }

    //db->tracer(odb::stderr_tracer);
    std::unique_ptr<odb::core::transaction> trans(
      new odb::core::transaction(db->begin()));

#ifdef DATABASE_SQLITE
    try
    {
      odb::schema_catalog::create_schema(*db, "", false);
      trans->commit();
    }
    catch (...)
    {
      // already exists
    }
#endif
  }
  catch (const odb::exception&)
  {
    // Connection failed!
    return nullptr;
  }

  return db;
}

} // stat
} // service
} // cc


