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

namespace cc
{
namespace util
{

std::shared_ptr<odb::database> createDatabase(const std::string& connStr_);

} // util
} // cc

#endif // CC_UTIL_DBUTIL_H
