#include <service/csharpservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace language
{

CsharpServiceHandler::CsharpServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
{
}

} // language
} // service
} // cc
