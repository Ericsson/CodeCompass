#ifndef __CODECOMPASS_SERVICES_SERVICE_TEST_H__
#define __CODECOMPASS_SERVICES_SERVICE_TEST_H__

#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <gtest/gtest.h>
#include <odb/database.hxx>

#include <util/util.h>
#include <util/filesystem.h>
#include <util/environment.h>

#include <memory>
#include <deque>
#include <fstream>
#include <algorithm>
#include <exception>

#include <sys/types.h>
#include <unistd.h>

/**
 * Helper macro for grocker::ServiceTestBase to add new databases to it.
 *
 * @param dbName database name in camel case
 * @param dbFileName database filename
 */
#define SERVICE_TEST_BASE_DB(dbName, dbFileName) \
  class dbName##DBOpener : public DBOpener \
  { \
  public: \
    const char* databaseName() const \
    { \
      return #dbName ; \
    } \
  protected: \
    void openDatabase(ServiceTestBase&                test_, \
                      std::shared_ptr<odb::database>& db_) \
    { \
      DBOpener::openDatabase(test_, dbFileName, db_); \
    } \
  }; \
  dbName##DBOpener dbName##Database;

namespace cc
{
namespace servicetest
{
  
namespace {

/**
 * Wrapper for getenv that throws an exception if the environment variable
 * is not set.
 */
const char *safe_getenv(const char *name)
{
  const char *ret = std::getenv(name);
  if (ret) {
    return ret;
  } else {
    throw std::runtime_error(
      std::string("Please set environment variable ") + name +
      " before starting this program.");
  }
}

}

/**
 * Base class for service tests.
 */
class ServiceTestBase : public ::testing::Test
{
public:
  /**
   * Database opener object
   */
  class DBOpener
  {
  protected:
    void openDatabase(ServiceTestBase&                test_,
                      const char*                     dbPath_,
                      std::shared_ptr<odb::database>& db_)
    {
      test_.openDatabase(dbPath_, db_);
    }

    virtual void openDatabase(ServiceTestBase&                test_,
                              std::shared_ptr<odb::database>& db_) = 0;

  public:
    std::shared_ptr<odb::database> operator()(ServiceTestBase* test)
    {
      std::shared_ptr<odb::database> db;
      openDatabase(*test, db);
      return db;
    }

    virtual const char* databaseName() const = 0;
  };

  friend class DBOpener;

public:
  class EnvironmentError : public std::exception
  {
  public:
    const char* what() const throw()
    {
      return "Necessary environment variable(s) is not set!";
    }
  };

public:
  ServiceTestBase() :
    _dbDir(safe_getenv("SERVICE_TEST_DB_DIR")),
    _testPid(::getpid())
  {
    if (_dbDir.empty())
    {
      throw new EnvironmentError;
    }

    util::Environment::init();
  }

  virtual ~ServiceTestBase() {}

protected:
  virtual void SetUp()
  {
    _dbToDelete.clear();
  }

  virtual void TearDown()
  {
    // clean databases (it might be modified)
    for (const std::string& fname : _dbToDelete)
    {
      ::unlink(fname.c_str());
    }
  }

  SERVICE_TEST_BASE_DB(Empty, "empty/empty.sqlite");
  SERVICE_TEST_BASE_DB(Simple, "simple/simple.sqlite");
  SERVICE_TEST_BASE_DB(TinyXml, "tinyxml/tinyxml.sqlite");
  SERVICE_TEST_BASE_DB(SearchMix, "searchmix/searchmix.sqlite");
  SERVICE_TEST_BASE_DB(TinyXmlWithGit, "tinyxmlwithgit/tinyxmlwithgit.sqlite");

private:
  /**
   * Creates a copy from the given database and then opens the copy and marks it
   * for tear down removal.
   *
   * @param dbPath_ the original database path relative to service database
   *                temporary directory.
   * @param db_ the result database
   */
  void openDatabase(std::string                     dbPath_,
                    std::shared_ptr<odb::database>& db_)
  {
    dbPath_ = util::pathFromDirectory(_dbDir, dbPath_);
    std::string tmpName(dbPath_ + ".test" + std::to_string(_testPid));

    // delete temp file if exists
    ::unlink(tmpName.c_str());

    // copy original db to test location
    backupFile(dbPath_, tmpName);

    _dbToDelete.push_back(tmpName);

    // create connection string
    std::string connString("sqlite:user=test_user;database=");
    connString += tmpName;

    // open db
    db_ = util::createDatabase(connString);
  }

  /**
   * Copies a file form one location to an another.
   *
   * @param from_ source location
   * @param to_ destination location
   */
  void backupFile(const std::string& from_, const std::string& to_)
  {
    std::ifstream source(from_.c_str(), std::ios::binary);
    std::ofstream dest(to_.c_str(), std::ios::binary);

    std::istreambuf_iterator<char> begin_source(source);
    std::istreambuf_iterator<char> end_source;
    std::ostreambuf_iterator<char> begin_dest(dest);
    std::copy(begin_source, end_source, begin_dest);
  }

protected:
  /**
   * Base path for the pregenerated test databases.
   */
  const std::string _dbDir;

  /**
   * Process id of the current test process.
   */
  const ::pid_t _testPid;

private:
  /**
   * Contains the temporary database paths. This databases are unnecessary after
   * running the test.
   */
  std::deque<std::string> _dbToDelete;
};

} // namespace servicetest
} // namespace grocker

#endif // __CODECOMPASS_SERVICES_SERVICE_TEST_H__
