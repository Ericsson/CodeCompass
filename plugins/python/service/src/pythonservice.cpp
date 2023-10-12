#include <service/pythonservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace language
{

PythonServiceHandler::PythonServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /*datadir_*/,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _context(context_)
{
}

} // python
} // service
} // cc
