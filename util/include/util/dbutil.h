#ifndef CC_UTIL_DBUTIL_H
#define CC_UTIL_DBUTIL_H

#include <memory>
#include <string>

#include <odb/database.hxx>

#ifdef DATABASE_PGSQL
#  define SQL_ILIKE "ILIKE"
#else
#  define SQL_ILIKE "LIKE"
#endif

// Regex search
#ifdef DATABASE_PGSQL
#  define SQL_REGEX "~*"
#else
#  define SQL_REGEX "REGEXP"
#endif

namespace cc
{
namespace util
{

std::shared_ptr<odb::database> createDatabase(const std::string& connStr_);

/**
 * This function adds indexes to the database. These indexes are added from the
 * .sql files which describe the model.
 * @param db_ Pointer to the ODB database.
 * @param sqlDir_ Directory path of SQL files.
 */
void createIndexes(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_);

/**
 * This function creates database tables. These tables are added from the .sql
 * files which describe the model.
 * @param db_ Pointer to the ODB database.
 * @param sqlDir_ Directory path of SQL files.
 */
void createTables(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_);

/**
 * This function removes database tables. These tables are removed based on the
 * .sql files which describe the model.
 * @param db_ Pointer to the ODB database.
 * @param sqlDir_ Directory path of SQL files.
 */
void removeTables(
  std::shared_ptr<odb::database> db_,
  const std::string& sqlDir_);

/**
 * This function updates a value for a given key in the connection string. The
 * connection string has the following format: dbsystem:key1=value1;key2=value2.
 * If the given key is not found in the connection string then it will be added
 * with the given value.
 */
std::string updateConnectionString(
  std::string connStr_,
  const std::string& key_,
  const std::string& value_);

/**
 * This function returns the value which belongs to the given key in the
 * connection string.
 */
std::string connStrComponent(
  const std::string& connStr_,
  const std::string& key_);

} // util
} // cc

#endif // CC_UTIL_DBUTIL_H
