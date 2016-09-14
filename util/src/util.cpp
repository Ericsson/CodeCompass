#include <util/util.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>

#include <boost/algorithm/string.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/regex.hpp>

#ifdef DATABASE_MYSQL
#include <odb/mysql/database.hxx>
#endif
#ifdef DATABASE_SQLITE
#include <odb/sqlite/database.hxx>
#endif
#ifdef DATABASE_PGSQL
#include <odb/pgsql/database.hxx>
#endif
#include <util/filesystem.h>
#include <util/streamlog.h>
#include <util/odbtransaction.h>

#include <model/project.h>
#include <model/project-odb.hxx>


namespace
{

class SQLTracer : public odb::tracer
{
  virtual void execute(odb::connection&, const char* statement_) override
  {
    SLog() << statement_;
  }
};

SQLTracer globalTracer;

std::vector<std::string> createOdbOptions(const std::string& connStr_)
{
  std::vector<std::string> connOpts;
  boost::split(connOpts, connStr_, boost::is_any_of(";"));

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

char** createCStyleOptions(
  const std::vector<std::string>& optionsVec,
  int& optsSize)
{
  optsSize = optionsVec.size();
  char **ret = new char*[optsSize + 1];

  for (unsigned i = 0; i < optionsVec.size(); ++i)
  {
    const auto& str = optionsVec[i];
    ret[i] = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), ret[i]);

    ret[i][str.size()] = '\0';
  }

  ret[optionsVec.size()] = 0;
  return ret;
}

void deleteCStyleOptions(char** options)
{
  int i = 0;
  while (options[i] != 0)
  {
    delete[] options[i++];
  }
  delete[] options;
}

#ifdef DATABASE_SQLITE

thread_local boost::regex sqliteRegex;
thread_local std::string  sqliteRegexExp;

void sqliteRegexImpl(
    ::sqlite3_context*  context_,
     int                argc_ ,
     ::sqlite3_value**  argv_)
{
  assert(argc_ == 2 && "Bad number of arguments!");

  try
  {
    const char* exprText = reinterpret_cast<const char*>(
        ::sqlite3_value_text(argv_[0]));
    if (sqliteRegexExp.compare(exprText) != 0)
    {
      sqliteRegexExp.assign(exprText);
      sqliteRegex.assign(exprText, boost::regex::icase);
    }

    const char* text = reinterpret_cast<const char*>(
        ::sqlite3_value_text(argv_[1]));
    if (boost::regex_search(text, sqliteRegex))
    {
      ::sqlite3_result_int(context_, 1);
    }
    else
    {
      ::sqlite3_result_int(context_, 0);
    }
  }
  catch (const boost::regex_error& err)
  {
    SLog(cc::util::ERROR) << "Regexp error: " << err.what();

    std::string msg = "Regexp exception: ";
    msg += err.what();
    ::sqlite3_result_error(context_, msg.c_str(), msg.size());
  }
}
#endif

} // anonymous namespace

namespace cc
{
namespace util
{

/**
 * Global connection string -> database map to avoid too many live connections.
 */
std::map<std::string, std::weak_ptr<odb::database>> databasePool;

std::shared_ptr<odb::database> createDatabase(const std::string& connStr)
{
  // Try to get from the pool
  {
    auto iter = databasePool.find(connStr);
    if (iter != databasePool.end())
    {
      auto db = iter->second.lock();
      if (db)
      {
        // Ok, we have it
        return db;
      }
      // expired
      databasePool.erase(iter);
    }
  }

  size_t colon = connStr.find(':');
  if (colon == std::string::npos)
  {
    return std::unique_ptr<odb::database>();
  }

  std::string database = connStr.substr(0, colon);
  std::string optionsStr = connStr.substr(colon + 1);

  auto odbOpts = createOdbOptions(optionsStr);
  int optionsSize;
  char **cStyleOptions = createCStyleOptions(odbOpts, optionsSize);

  std::shared_ptr<odb::database> db;

#ifdef DATABASE_MYSQL
  if (database == "mysql")
  {
    db.reset(new odb::mysql::database(optionsSize, cStyleOptions));
  }
#endif
#ifdef DATABASE_SQLITE
  if (database == "sqlite")
  {
    auto sqliteDB = new odb::sqlite::database(
      optionsSize,
      cStyleOptions,
      false,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
      true,
      "",
      std::make_unique<odb::sqlite::single_connection_factory>());
    db.reset(sqliteDB);

    auto sqlitePtr = sqliteDB->connection()->handle();
    ::sqlite3_create_function_v2(
      sqlitePtr,
      "regexp", 2,  // regexp function with one argument
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
    db.reset(new odb::pgsql::database(optionsSize, cStyleOptions));
  }
#endif

  deleteCStyleOptions(cStyleOptions);

  // Test database connection
  try
  {
    if (!db)
    {
      return nullptr;
    }

    //db->tracer(globalTracer);

    std::unique_ptr<odb::core::transaction> trans(
      new odb::core::transaction(db->begin()));
  }
  catch (const odb::exception&)
  {
    // Connection failed!
    return nullptr;
  }

  databasePool[connStr] = db;
  return db;
}

uint64_t fnvHash(const std::string& data)
{
  uint64_t hash = 14695981039346656037ULL;

  auto len = data.length();
  for (unsigned int i = 0; i < len; ++i)
  {
    hash ^= static_cast<uint64_t>(data[i]);
    hash *= static_cast<uint64_t>(1099511628211ULL);
  }

  return hash;
}

std::string generateSha1Hash(const std::string& data_)
{
  using namespace boost::uuids::detail;

  sha1 hasher;
  unsigned int digest[5];

  hasher.process_bytes(data_.c_str(), data_.size());
  hasher.get_digest(digest);

  std::stringstream ss;
  ss.setf(std::ios::hex, std::ios::basefield);
  ss.width(8);
  ss.fill('0');

  for (int i = 0; i < 5; ++i)
  {
    ss << digest[i];
  }

  return ss.str();
}

/*
const char SLASH = '/';
 */

bool isExtension(const std::string& path, const std::string& ext)
{
  return getExtension(path) == ext;
}

std::string getFilename(const std::string& path)
{
  /*
  std::size_t pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos+1);
   */
  return getPathFilename(path);
}

std::string getExtension(const std::string& filename)
{
  /*
  std::size_t pos = filename.rfind('.');
  return pos == std::string::npos ? "" : filename.substr(pos+1);
   */
  return getPathExtension(filename);
}

bool isCppFile(const std::string& path_){
  std::string ext = util::getExtension(path_);
  return ext == "cc"  || ext == "cpp"  || ext == "cxx" ||
         ext == "cp" || ext == "CPP" || ext == "c++" ||
         ext == "C"   || ext == "c";  //last one for C
}

std::string getPathAndFileWithoutExtension(const std::string& path)
{
  /*
  std::size_t pos = path.rfind('.');
  return pos == std::string::npos ? path : path.substr(0,pos);
   */
  std::string ext(getPathExtension(path));
  if (!ext.empty())
  {
    return path.substr(0, path.size() - getPathExtension(path).size() - 1);
  }
  else
  {
    return path;
  }
}

std::string getPath(const std::string& path)
{
  /*
  std::size_t pos = path.rfind(SLASH);
  return pos == std::string::npos ? "" : path.substr(0, pos+1);
   */
  return getPathParent(path);
}

FileType getFileType(const std::string& path, FileType defaultType)
{
  std::string extension = getExtension(path);
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  // Some binary files have no extension.
  // Since util::getFilename and util::getExtension both returns the filename
  // for files without extension, it's quite hard to decide whether a file has
  // a real extension or not.

  if (path == getPathAndFileWithoutExtension(path) + "/" + getFilename(path))
    return defaultType;
  else if (extension == "cpp" || extension == "cxx" || extension == "cc"  || extension == "c")
    return FileType::SOURCE;
  else if(extension == "hpp" || extension == "hxx" || extension == "hh"  || extension == "h")
    return FileType::HEADER;
  else if (extension == "o"  || extension == "so" || extension == "dll")
    return FileType::OBJECT;
  else if(extension == "out" || extension == "exe")
    return FileType::BINARY;
  return defaultType;
}

bool isFileExist(const std::string& fn_)
{
  return access(fn_.c_str(), F_OK)==0;
}

/**
 * Monotonic timer using milliseconds
 */
long long int getTickCount()
{
  auto p = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch()).count();
}

} // util
} // cc
