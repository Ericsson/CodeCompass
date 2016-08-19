#ifndef CC_UTIL_DBUTIL_H
#define CC_UTIL_DBUTIL_H

#include <memory>
#include <string>

#include <odb/database.hxx>

namespace cc
{
namespace util
{

std::shared_ptr<odb::database> createDatabase(const std::string& connStr_);

} // util
} // cc

#endif // CC_UTIL_DBUTIL_H
