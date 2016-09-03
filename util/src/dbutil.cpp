#include <map>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>

#ifdef DATABASE_SQLITE
#  include <odb/sqlite/database.hxx>
#endif

#ifdef DATABASE_PGSQL
#  include <odb/pgsql/database.hxx>
#endif

#include <util/dbutil.h>

namespace
{

std::vector<std::string> createOdbOptions(const std::string& connStr_)
{
  std::vector<std::string> connOpts;
  boost::split(connOpts, connStr_, boost::is_any_of(";"));

  std::vector<std::string> opts = {" "}; // For some reason ODB needs it
  for (std::string opt : connOpts)
  {
    if (opt.empty())
      continue;

    std::string val;
    std::size_t assignPos = opt.find('=');

    if (assignPos != std::string::npos)
    {
      val = opt.substr(assignPos + 1);
      opt = opt.substr(0, assignPos);
    }

#ifdef DATABASE_PGSQL
    if (opt == "passwdfile")
    {
      setenv("PGPASSFILE", val.c_str(), 1);
      continue;
    }
#endif

    opts.emplace_back("--" + opt);
    if (!val.empty())
      opts.emplace_back(std::move(val));
  }

  return opts;
}

/**
 * This function transforms a string vector to a C-style string array. The
 * returning array has the length of optionsVec_ + 1. The last element of the
 * array is a null pointer. This helps the deleteCStyleOptions() function to
 * find the end of the array. The array contains C-style strings with
 * terminating null character.
 */
char** createCStyleOptions(const std::vector<std::string>& optionsVec_)
{
  char** ret = new char*[optionsVec_.size() + 1];

  for (std::size_t i = 0; i < optionsVec_.size(); ++i)
  {
    const std::string& str = optionsVec_[i];
    ret[i] = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), ret[i]);
    ret[i][str.size()] = '\0';
  }

  ret[optionsVec_.size()] = 0;

  return ret;
}

/**
 * This function deallocates the array created by createCStyleOptions()
 * function.
 */
void deleteCStyleOptions(char** options)
{
  for (int i = 0; options[i]; ++i)
    delete[] options[i];
  delete[] options;
}

#ifdef DATABASE_SQLITE
void sqliteRegexImpl(
  sqlite3_context* context_,
  int argc_,
  sqlite3_value** argv_)
{
  assert(argc_ == 2 && "Bad number of arguments!");

  try
  {
    boost::regex sqliteRegex;
    std::string sqliteRegexExp;

    const char* exprText = reinterpret_cast<const char*>(
      sqlite3_value_text(argv_[0]));
    if (sqliteRegexExp.compare(exprText) != 0)
    {
      sqliteRegexExp.assign(exprText);
      sqliteRegex.assign(exprText, boost::regex::icase);
    }

    const char* text = reinterpret_cast<const char*>(
      sqlite3_value_text(argv_[1]));
    sqlite3_result_int(context_, boost::regex_search(text, sqliteRegex));
  }
  catch (const boost::regex_error& err)
  {
    std::string msg = "Regexp exception: ";
    msg += err.what();
    sqlite3_result_error(context_, msg.c_str(), msg.size());
  }
}
#endif

}

namespace cc
{
namespace util
{

/**
 * Global connection string -> database map to avoid too many live connections.
 */
std::map<std::string, std::shared_ptr<odb::database>> databasePool;

std::shared_ptr<odb::database> createDatabase(const std::string& connStr_)
{
  auto iter = databasePool.find(connStr_);
  if (iter != databasePool.end())
  {
    auto db = iter->second;
    
    if (db)
      return db;

    databasePool.erase(iter);
  }

  std::size_t colonPos = connStr_.find(':');
  if (colonPos == std::string::npos)
    return std::shared_ptr<odb::database>();

  std::string database = connStr_.substr(0, colonPos);
  std::string options = connStr_.substr(colonPos + 1);

  std::vector<std::string> odbOpts = createOdbOptions(options);
  char** cStyleOptions = createCStyleOptions(odbOpts);

  std::shared_ptr<odb::database> db;

#ifdef DATABASE_MYSQL
  if (database == "mysql")
    db.reset(new odb::mysql::database(odbOpts.size(), cStyleOptions),
      [](odb::database*){});
#endif

#ifdef DATABASE_SQLITE
  if (database == "sqlite")
  {
    int optionsSize = odbOpts.size();
    auto sqliteDB = new odb::sqlite::database(
      optionsSize,
      cStyleOptions,
      false,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
      true,
      "",
      std::make_unique<odb::sqlite::single_connection_factory>());
    db.reset(sqliteDB, [](odb::database*){});

    auto sqlitePtr = sqliteDB->connection()->handle();
    sqlite3_create_function_v2(
      sqlitePtr,
      "regexp", 2, // regexp function with one argument
      SQLITE_UTF8,
      nullptr,
      &sqliteRegexImpl,
      nullptr,
      nullptr,
      nullptr);
  }
#endif

#ifdef DATABASE_PGSQL
  if (database == "pgsql")
  {
    int size = odbOpts.size();
    db.reset(new odb::pgsql::database(size, cStyleOptions),
      [](odb::database*){});
  }
#endif

  deleteCStyleOptions(cStyleOptions);

  // Test connection
  try
  {
    if (!db)
      return nullptr;

    std::unique_ptr<odb::core::transaction> trans(
      new odb::core::transaction(db->begin()));
  }
  catch (const odb::exception& ex)
  {
    BOOST_LOG_TRIVIAL(error) << ex.what();
    return nullptr;
  }

  databasePool[connStr_] = db;

  return db;
}

} // util
} // cc
