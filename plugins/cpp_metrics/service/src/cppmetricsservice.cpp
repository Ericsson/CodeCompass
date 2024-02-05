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
  typeName.name = "Lack of cohesion HS of function";
  _return.push_back(typeName);
}

void CppMetricsServiceHandler::getCppMetricsForAstNode(
  std::vector<CppMetricsAstNode>& _return,
  const core::AstNodeId& astNodeId_)
{
  std::vector<CppMetricsTypeName> types;
  getCppMetricsTypeNames(types);
  CppMetricsAstNode metric;

  for (const auto& pair : types)
  {
    double value = astNodeMetrics(astNodeId_, pair.type);
    metric.type = pair.type;
    metric.value = value;
    _return.push_back(metric);
  }
}

double CppMetricsServiceHandler::getSingleCppMetricForAstNode(
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
      return -DBL_MAX;

    return nodeMetric.begin()->value;
  });
}

} // cppmetrics
} // service
} // cc
