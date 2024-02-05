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

double CppMetricsServiceHandler::getCppMetricsForAstNode(
  const core::AstNodeId& astNodeId_,
  const CppMetricsType::type metrics_)
{
  return astNodeMetrics(astNodeId_, metrics_);
}

double CppMetricsServiceHandler::astNodeMetrics(
  const ::cc::service::core::AstNodeId& astNodeId_,
  const CppMetricsType::type type_)
{
  _transaction([&, this](){
    typedef odb::result<model::CppAstNode> CppAstNodeResult;
    typedef odb::query<model::CppAstNode> CppAstNodeQuery;
    typedef odb::result<model::CppAstNodeMetrics> CppAstNodeMetricsResult;
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;

    auto nodeMetric = _db->query<model::CppAstNodeMetrics>(
      CppAstNodeMetricsQuery::astNodeId == std::stoull(astNodeId_) &&
      CppAstNodeMetricsQuery::type ==
        static_cast<model::CppAstNodeMetrics::Type>(type_));

    if (nodeMetric.empty())
      return -1.0;

    return nodeMetric.begin()->value;
  });
}

} // cppmetrics
} // service
} // cc
