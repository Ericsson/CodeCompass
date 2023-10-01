#include <service/cppmetricsservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace cppmetrics
{

CppMetricsServiceHandler::CppMetricsServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /*datadir_*/,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
}

void CppMetricsServiceHandler::getDummyString(std::string& str_)
{
  str_ = _config["dummy-result"].as<std::string>();
}

} // cppmetrics
} // service
} // cc
