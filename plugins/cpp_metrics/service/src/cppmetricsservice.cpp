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
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
}

void CppMetricsServiceHandler::getCppMetricsTypeNames(
  std::vector<CppMetricsTypeName>& _return)
{
  CppMetricsTypeName typeName;

  typeName.type = CppMetricsType::ParameterCount;
  typeName.name = "Number of function parameters";
  _return.push_back(typeName);

  typeName.type = CppMetricsType::McCabe;
  typeName.name = "McCabe metric";
  _return.push_back(typeName);

  typeName.type = CppMetricsType::LackOfCohesion;
  typeName.name = "Lack of cohesion of function";
  _return.push_back(typeName);

  typeName.type = CppMetricsType::LackOfCohesionHS;
  typeName.name = "Lack of cohesion of function (Henderson-Sellers variant)";
  _return.push_back(typeName);
}

void CppMetricsServiceHandler::getCppMetricsForAstNode(
  std::vector<CppMetricsAstNode>& _return,
  const core::AstNodeId& astNodeId_)
{
  CppMetricsAstNode metric;

  _transaction([&, this](){
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;

    auto nodeMetrics = _db->query<model::CppAstNodeMetrics>(
      CppAstNodeMetricsQuery::astNodeId == std::stoull(astNodeId_));

    for (const auto& nodeMetric : nodeMetrics)
    {
      metric.type = static_cast<CppMetricsType::type>(nodeMetric.type);
      metric.value = nodeMetric.value;
      _return.push_back(metric);
    }
  });
}

double CppMetricsServiceHandler::getSingleCppMetricForAstNode(
  const core::AstNodeId& astNodeId_,
  CppMetricsType::type metric_)
{
  return _transaction([&, this]() -> std::double_t {
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;

    auto nodeMetric = _db->query<model::CppAstNodeMetrics>(
      CppAstNodeMetricsQuery::astNodeId == std::stoull(astNodeId_) &&
      CppAstNodeMetricsQuery::type == static_cast<model::CppAstNodeMetrics::Type>(metric_));

    if (nodeMetric.empty())
      return -DBL_MAX;

    return nodeMetric.begin()->value;
  });
}

} // cppmetrics
} // service
} // cc
