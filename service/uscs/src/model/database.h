#ifndef __CC_USCS_MODEL_DATABASE_H__
#define __CC_USCS_MODEL_DATABASE_H__

#include <memory>
#include <string>

#include <odb/database.hxx>

namespace cc
{
namespace service
{
namespace stat
{

/**
 * Opens the database by the given connection string.
 *
 * @param conn_ a connection string.
 * @return a database instance or null on error.
 */
std::shared_ptr<odb::database> openDatabase(const std::string& conn_);

} // stat
} // service
} // cc

#endif // __CC_USCS_MODEL_DATABASE_H__

