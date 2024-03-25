#include <service/cppmetricsservice.h>
#include <util/dbutil.h>

#include <odb/query.hxx>

namespace cc
{
namespace service
{
namespace cppmetrics
{

typedef odb::query<cc::model::CppAstNode> AstQuery;
typedef odb::result<cc::model::CppAstNode> AstResult;

CppMetricsServiceHandler::CppMetricsServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
}

void CppMetricsServiceHandler::getCppAstNodeMetricsTypeNames(
  std::vector<CppAstNodeMetricsTypeName>& _return)
{
  CppAstNodeMetricsTypeName typeName;

  typeName.type = CppAstNodeMetricsType::ParameterCount;
  typeName.name = "Number of function parameters";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::McCabe;
  typeName.name = "McCabe metric";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::LackOfCohesion;
  typeName.name = "Lack of cohesion of function";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::LackOfCohesionHS;
  typeName.name = "Lack of cohesion of function (Henderson-Sellers variant)";
  _return.push_back(typeName);
}

void CppMetricsServiceHandler::getCppModuleMetricsTypeNames(
  std::vector<CppModuleMetricsTypeName>& _return)
{
  CppModuleMetricsTypeName typeName;

  typeName.type = CppModuleMetricsType::Placeholder;
  typeName.name = "Placeholder";
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
      metric.type = static_cast<CppAstNodeMetricsType::type>(nodeMetric.type);
      metric.value = nodeMetric.value;
      _return.push_back(metric);
    }
  });
}

double CppMetricsServiceHandler::getSingleCppMetricForAstNode(
  const core::AstNodeId& astNodeId_,
  CppAstNodeMetricsType::type metric_)
{
  return _transaction([&, this]() -> std::double_t {
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;

    auto nodeMetric = _db->query<model::CppAstNodeMetrics>(
      CppAstNodeMetricsQuery::astNodeId == std::stoull(astNodeId_) &&
      CppAstNodeMetricsQuery::type == static_cast<model::CppAstNodeMetrics::Type>(metric_));

    if (nodeMetric.empty())
    {
      core::InvalidInput ex;
      ex.__set_msg("Invalid metric type for AST node: " + astNodeId_);
      throw ex;
    }

    return nodeMetric.begin()->value;
  });
}

void CppMetricsServiceHandler::getCppMetricsForModule(
  std::vector<CppMetricsModule>& _return,
  const core::FileId& fileId_)
{
  CppMetricsModule metric;

  _transaction([&, this](){
    typedef odb::query<model::CppFileMetrics> CppModuleMetricsQuery;

    auto moduleMetrics = _db->query<model::CppFileMetrics>(
      CppModuleMetricsQuery::file == std::stoull(fileId_));

    for (const auto& moduleMetric : moduleMetrics)
    {
      metric.type = static_cast<CppModuleMetricsType::type>(moduleMetric.type);
      metric.value = moduleMetric.value;
      _return.push_back(metric);
    }
  });
}

void CppMetricsServiceHandler::getCppAstNodeMetricsForPath(
  std::vector<CppAllMetricsAstNode>& _return,
  const std::string& path_)
{
  _transaction([&, this]()
  {
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;
    typedef odb::result<model::CppAstNodeMetrics> CppAstNodeMetricsResult;
    typedef odb::query<model::CppAstNodeFilePath> CppAstNodeFilePathQuery;
    typedef odb::result<model::CppAstNodeFilePath> CppAstNodeFilePathResult;

    CppAstNodeFilePathResult nodes = _db->query<model::CppAstNodeFilePath>(
      CppAstNodeFilePathQuery::LocFile::path.like(path_ + '%'));

    if (nodes.empty())
    {
      core::InvalidInput ex;
      ex.__set_msg("Invalid metric type for path: " + path_);
      throw ex;
    }

    for (const auto& node : nodes)
    {
      auto metricsQuery = _db->query<model::CppAstNodeMetrics>(
        CppAstNodeMetricsQuery::astNodeId == node.id);
      std::vector<CppMetricsAstNode> metrics;

      CppMetricsAstNode metricsAstNode;
      for (const auto& metric : metricsQuery)
      {
        metricsAstNode.type = static_cast<CppAstNodeMetricsType::type>(metric.type);
        metricsAstNode.value = metric.value;
      }

      CppAllMetricsAstNode nodeMetric;
      nodeMetric.id = std::to_string(node.id);
      nodeMetric.metrics = metrics;
      _return.push_back(nodeMetric);
    }
  });
}

void CppMetricsServiceHandler::getCppFileMetricsForPath(
  std::vector<CppAllMetricsModule>& _return,
  const std::string& path_)
{
  _transaction([&, this]()
  {
    typedef odb::query<model::File> FileQuery;
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::CppFileMetrics> CppFileMetricsQuery;
    typedef odb::result<model::CppFileMetrics> CppFileMetricsResult;

    FileResult descendants = _db->query<model::File>(
      FileQuery::path.like(path_ + '%'));

    if (descendants.empty())
    {
      core::InvalidInput ex;
      ex.__set_msg("Invalid metric type for path: " + path_);
      throw ex;
    }

    for (const auto& file : descendants)
    {
      CppFileMetricsResult metricsQuery = _db->query<model::CppFileMetrics>(
        CppFileMetricsQuery::file == file.id);
      std::vector<CppMetricsModule> metrics;

      CppMetricsModule metricsModule;
      for (const auto& metric : metricsQuery)
      {
        metricsModule.type = static_cast<CppModuleMetricsType::type>(metric.type);
        metricsModule.value = metric.value;
      }

      CppAllMetricsModule nodeMetric;
      nodeMetric.id = std::to_string(file.id);
      nodeMetric.metrics = metrics;
      _return.push_back(nodeMetric);
    }
  });
}

} // cppmetrics
} // service
} // cc
