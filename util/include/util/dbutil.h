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
/**
 * This function connects to and if required, optionally creates a database.
 * @param connStr_ The database connection string.
 * @param create_ True to create database if does not exist; otherwise, false.
 */
std::shared_ptr<odb::database> connectDatabase(
  const std::string& connStr_,
  bool create_ = true);

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

/**
 * This function returns the database driver name being used. Currently "pgsql"
 * or "sqlite" can be returned depending on the build option giving which
 * database system to use.
 */
inline std::string getDbDriver()
{
#if defined(DATABASE_PGSQL)
  return "pgsql";
#elif defined(DATABASE_SQLITE)
  return "sqlite";
#else
  return "";
#endif
}

/// @brief Determines if the specified ODB query result only contains
/// a single entity. That single entity is then stored in 'single_'.
/// @tparam TEntity The type of entities in the query result.
/// @param result_ The ODB query result in question.
/// @param single_ The variable that receives the first entity (if any).
/// @return Returns true if 'result_' only contained 'single_';
/// otherwise false.
template<typename TEntity>
bool isSingleResult(odb::result<TEntity>& result_, TEntity& single_)
{
  auto it_b = result_.begin();
  const auto it_e = result_.end();
  if (it_b != it_e)
  {
    single_ = *it_b;
    return ++it_b == it_e;
  }
  else return false;
}

/// @brief Determines if the specified ODB query result only contains
/// a single entity.
/// @tparam TEntity The type of entities in the query result.
/// @param result_ The ODB query result in question.
/// @return Returns true if 'result_' only contained a single entity;
/// otherwise false.
template<typename TEntity>
bool isSingleResult(odb::result<TEntity>& result_)
{
  auto it_b = result_.begin();
  const auto it_e = result_.end();
  return (it_b != it_e) && (++it_b == it_e);
}

/// @brief Constructs an ODB query that you can use to filter only
/// the database records of the given parameter type whose path
/// is rooted under any of the specified filter paths.
/// @tparam TQueryParam The type of database records to query.
/// This type must represent an ODB view that has access to
/// (i.e. is also joined with) the File table.
/// @tparam TIter The iterator type of the filter paths.
/// @tparam TSentinel The type of the end of the filter paths.
/// @param begin_ The iterator referring to the first filter path.
/// @param end_ The sentinel for the end of the filter paths.
/// @return A query containing the disjunction of filters.
template<typename TQueryParam, typename TIter, typename TSentinel>
odb::query<TQueryParam> getFilterPathsQuery(
  TIter begin_,
  const TSentinel& end_)
{
  typedef typename odb::query<TQueryParam>::query_columns QParam;
  const auto& QParamPath = QParam::File::path;
  constexpr char ODBWildcard = '%';

  assert(begin_ != end_ && "At least one filter path must be provided.");
  odb::query<TQueryParam> query = QParamPath.like(*begin_ + ODBWildcard);
  while (++begin_ != end_)
    query = query || QParamPath.like(*begin_ + ODBWildcard);
  return query;
}

} // util
} // cc

#endif // CC_UTIL_DBUTIL_H
