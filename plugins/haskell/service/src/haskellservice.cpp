#include <service/haskellservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace haskell
{

HaskellServiceHandler::HaskellServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /*datadir_*/,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
}

} // haskell
} // service
} // cc
