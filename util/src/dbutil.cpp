#include <fstream>
#include <map>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#ifdef DATABASE_SQLITE
#  include <odb/sqlite/database.hxx>
#endif

#ifdef DATABASE_PGSQL
#  include <odb/pgsql/database.hxx>
#endif

#include <odb/connection.hxx>

#include <util/logutil.h>
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

#ifdef DATABASE_PGSQL
/**
 * This function checks the existance of a postgres database and
 * optionally creates the database if it doesn't exists.
 * @return True if the database exists after the function call; otherwise, false.
 */
bool checkPsqlDatbase(const std::string& connStr_, const std::string dbName_, bool create_)
{
  std::size_t colonPos = connStr_.find(':');
  std::string database = connStr_.substr(0, colonPos);
  std::string options  = connStr_.substr(colonPos + 1);

  std::vector<std::string> odbOpts = createOdbOptions(options);
  char** cStyleOptions = createCStyleOptions(odbOpts);

  int size = odbOpts.size();
  odb::pgsql::database db(size, cStyleOptions);

  odb::connection_ptr connection = db.connection();
  try
  {
    bool exists = connection->execute(
      "SELECT 1 FROM pg_database WHERE datname = '" + dbName_ + "'") > 0;

    if(!exists && create_)
    {
      std::string createCmd = "CREATE DATABASE " + dbName_
        + " ENCODING = 'SQL_ASCII'"
        + " LC_CTYPE='C'"
        + " LC_COLLATE='C'"
        + " TEMPLATE template0;";

      connection->execute(createCmd);

      LOG(info) << "Creating database: " << dbName_;
    }

    exists = connection->execute(
      "SELECT 1 FROM pg_database WHERE datname = '" + dbName_ + "'") > 0;

    return exists;
  }
  catch (const odb::exception& ex)
  {
    LOG(warning) << ex.what();
  }
}
#endif

/**
 * This function removes all parts from the s_ string between begin_ and end_.
 * For example removeByRegex("abc(def)ghi", "(", ")") == "abcghi";
 */
std::string removeByRegex(
  const std::string& s_,
  const std::string& begin_,
  const std::string& end_)
{
  boost::regex expr(begin_ + "([\\s\\S]*?)" + end_ + "\\n?");
  return boost::regex_replace(s_, expr, "");
}

/**
 * This function runs all .sql files which are produced by ODB.
 * @param db_ A database object.
 * @param sqlDir_ Path of the directory containing .sql files.
 * @param replacer_ A function can be given which will be applied to the .sql
 * file content before execution.
 * @param logMessage_ This log message is printed before the .sql file
 * execution followed by the file name.
 */
void runSqlFiles(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_,
  std::function<std::string(const std::string&)> replacer_,
  const std::string& logMessage_)
{
  odb::connection_ptr connection = db_->connection();

  for (
    boost::filesystem::directory_iterator it(sqlDir_);
    it != boost::filesystem::directory_iterator();
    ++it)
  {
    if (!boost::filesystem::is_regular_file(it->path()))
      continue;

    std::string fileName = boost::filesystem::canonical(it->path()).native();
    LOG(info) << logMessage_ << ' ' << fileName;

    std::ifstream file(fileName);

    std::string fileContent(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

    file.close();

    try
    {
      // In SQLite if several SQL commands are provided separated by semicolon
      // then only the first executes. So we have to split and execute them one
      // by one.
      std::string sql = replacer_(fileContent);
      std::vector<std::string> v;
      boost::algorithm::split_regex(v, sql, boost::regex("\n\n"));

      for (std::size_t i = 0; i < v.size(); ++i)
      {
#ifdef DATABASE_SQLITE
        // DROP TABLE SQL commands generated by ODB may contain "CASCADE"
        // keyword which is not known by SQLITE.
        if (v[i].find("DROP TABLE") == 0)
        {
          std::size_t pos = v[i].find("CASCADE");
          if (pos != std::string::npos)
            v[i].erase(pos, 7); // 7 == length of "CASCADE"
        }
#endif
        connection->execute(v[i]);
      }
    }
    catch (const odb::exception& ex)
    {
      LOG(warning) << "Exception when running SQL command: " << ex.what();
    }
  }
}

}

namespace cc
{
namespace util
{

/**
 * Global connection string -> database map to avoid too many live connections.
 */
static std::map<std::string, std::shared_ptr<odb::database>> databasePool;

std::shared_ptr<odb::database> connectDatabase(const std::string& connStr_, bool create_)
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
  int optionsSize = odbOpts.size();

  std::shared_ptr<odb::database> db;

#ifdef DATABASE_SQLITE
  if (database == "sqlite")
  {
    try
    {
      auto sqliteDB = new odb::sqlite::database(
        optionsSize,
        cStyleOptions,
        false,
        SQLITE_OPEN_READWRITE | (create_ ? SQLITE_OPEN_CREATE : 0),
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
    catch (odb::database_exception& e)
    {
      // Could not connect to DB
      db.reset();
    }
  }
#endif

#ifdef DATABASE_PGSQL
  if (database == "pgsql")
  {
    std::string defaultPsqlConnStr =
      updateConnectionString(connStr_, "database", "postgres");
    std::string dbName = connStrComponent(connStr_, "database");

    if (checkPsqlDatbase(defaultPsqlConnStr, dbName, create_))
    {
      db.reset(new odb::pgsql::database(optionsSize, cStyleOptions),
               [](odb::database*) {});
    }
    else
    {
      // DB does not exists
      db.reset();
    }
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
    LOG(error) << ex.what();
    return nullptr;
  }

  databasePool[connStr_] = db;

  return db;
}

void createTables(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_)
{
#ifdef DATABASE_SQLITE
  db_->connection()->execute("PRAGMA foreign_keys = ON");
#endif
  runSqlFiles(db_, sqlDir_,
    [](const std::string& s_){
      return removeByRegex(removeByRegex(removeByRegex(removeByRegex(s_,
        "CREATE UNIQUE INDEX", ";"),
        "CREATE INDEX", ";"),
        "ALTER TABLE", ";"),
        "DROP", ";");
    },
    "Creating tables from file");
}

void removeTables(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_)
{
#ifdef DATABASE_SQLITE
  db_->connection()->execute("PRAGMA foreign_keys = OFF");
#endif
  runSqlFiles(db_, sqlDir_,
    [](const std::string& s_){
      return removeByRegex(removeByRegex(s_,
        "CREATE", ";"),
        "ALTER TABLE", ";");
    },
    "Dropping tables from file");
}

void createIndexes(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_)
{
  runSqlFiles(db_, sqlDir_,
    [](const std::string& s_){
      return removeByRegex(removeByRegex(s_,
        "CREATE TABLE", ";"),
        "DROP", ";");
    },
    "Creating indexes from file");
}

std::string updateConnectionString(
  std::string connStr_,
  const std::string& key_,
  const std::string& value_)
{
  std::string prefix = key_ + '=';
  std::size_t pos1 = connStr_.find(prefix);

  if (pos1 == std::string::npos)
  {
    if (connStr_.back() != ':' && !connStr_.empty())
      connStr_.push_back(';');
    connStr_.append(prefix + value_);
  }
  else
  {
    std::size_t pos2 = connStr_.find(';', pos1 + 1);
    if (pos2 == std::string::npos)
      pos2 = connStr_.length();

    int pos3 = pos1 + prefix.length();
    connStr_.replace(pos3, pos2 - pos3, value_);
  }

  return connStr_;
}

std::string connStrComponent(
  const std::string& connStr_,
  const std::string& key_)
{
  std::string prefix = key_ + '=';
  std::size_t pos1 = connStr_.find(prefix);

  if (pos1 == std::string::npos)
    return std::string();

  std::size_t pos2 = connStr_.find(';', pos1 + 1);
  std::size_t pos3 = pos1 + prefix.length();

  if (pos2 == std::string::npos)
    pos2 = connStr_.length();

  return connStr_.substr(pos3, pos2 - pos3);
}

} // util
} // cc
