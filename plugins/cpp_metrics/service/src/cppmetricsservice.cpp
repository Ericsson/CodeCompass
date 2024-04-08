#include <service/cppmetricsservice.h>
#include <util/dbutil.h>

#include <odb/query.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

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
  std::vector<CppMetricsAstNodeSingle>& _return,
  const core::AstNodeId& astNodeId_)
{
  CppMetricsAstNodeSingle metric;

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
  std::vector<CppMetricsModuleSingle>& _return,
  const core::FileId& fileId_)
{
  CppMetricsModuleSingle metric;

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
  std::vector<CppMetricsAstNodeAll>& _return,
  const std::string& path_)
{
  _transaction([&, this](){
    typedef odb::query<model::CppAstNodeMetrics> CppAstNodeMetricsQuery;
    typedef odb::result<model::CppAstNodeMetrics> CppAstNodeMetricsResult;
    typedef odb::query<model::CppAstNodeFilePath> CppAstNodeFilePathQuery;
    typedef odb::result<model::CppAstNodeFilePath> CppAstNodeFilePathResult;

    auto nodesRes = _db->query<model::CppAstNodeMetrics>();
    std::set<model::CppAstNodeId> nodesWithMetrics;
    for (const auto& node : nodesRes)
      nodesWithMetrics.insert(node.astNodeId);

    CppAstNodeFilePathResult nodes = _db->query<model::CppAstNodeFilePath>(
      CppAstNodeFilePathQuery::LocFile::path.like(path_ + '%') &&
      CppAstNodeFilePathQuery::CppAstNode::id.in_range(
        nodesWithMetrics.begin(), nodesWithMetrics.end()));

    if (nodes.empty())
      return;

    for (const auto& node : nodes)
    {
      auto metricsQuery = _db->query<model::CppAstNodeMetrics>(
        CppAstNodeMetricsQuery::astNodeId == node.id);
      std::vector<CppMetricsAstNodeSingle> metrics;

      CppMetricsAstNodeSingle metricsAstNode;
      for (const auto& metric : metricsQuery)
      {
        metricsAstNode.type = static_cast<CppAstNodeMetricsType::type>(metric.type);
        metricsAstNode.value = metric.value;
        metrics.push_back(metricsAstNode);
      }

      if (metrics.empty())
        continue;

      CppMetricsAstNodeAll nodeMetric;
      nodeMetric.id = std::to_string(node.id);
      nodeMetric.metrics = metrics;
      _return.push_back(nodeMetric);
    }
  });
}

void CppMetricsServiceHandler::getCppFileMetricsForPath(
  std::vector<CppMetricsModuleAll>& _return,
  const std::string& path_)
{
  _transaction([&, this](){
    typedef odb::query<model::FilePathView> FileQuery;
    typedef odb::result<model::FilePathView> FileResult;
    typedef odb::query<model::CppFileMetrics> CppFileMetricsQuery;
    typedef odb::result<model::CppFileMetrics> CppFileMetricsResult;

    auto filesRes = _db->query<model::CppFileMetrics>();
    std::set<model::CppAstNodeId> filesWithMetrics;
    for (const auto& file : filesRes)
      filesWithMetrics.insert(file.file);

    FileResult descendants = _db->query<model::FilePathView>(
      FileQuery::path.like(path_ + '%') &&
      CppFileMetricsQuery::file.in_range(
        filesWithMetrics.begin(), filesWithMetrics.end()));

    if (descendants.empty())
      return;

    for (const auto& file : descendants)
    {
      CppFileMetricsResult metricsQuery = _db->query<model::CppFileMetrics>(
        CppFileMetricsQuery::file == file.id);
      std::vector<CppMetricsModuleSingle> metrics;

      CppMetricsModuleSingle metricsModule;
      for (const auto& metric : metricsQuery)
      {
        metricsModule.type = static_cast<CppModuleMetricsType::type>(metric.type);
        metricsModule.value = metric.value;
        metrics.push_back(metricsModule);
      }

      if (metrics.empty())
        continue;

      CppMetricsModuleAll nodeMetric;
      nodeMetric.id = std::to_string(file.id);
      nodeMetric.metrics = metrics;
      _return.push_back(nodeMetric);
    }
  });
}

} // cppmetrics
} // service
} // cc
