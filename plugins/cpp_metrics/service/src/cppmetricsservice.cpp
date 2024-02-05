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

//double CppMetricsServiceHandler::getCppMetricsForAstNode(
  //const core::AstNodeId& astNodeId_,
  //const CppMetricsType::type metrics_)
//{

//}

} // cppmetrics
} // service
} // cc
