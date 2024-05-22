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
  typeName.name = "Cyclomatic (McCabe) complexity of function";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::BumpyRoad;
  typeName.name = "Bumpy road complexity of function";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::LackOfCohesion;
  typeName.name = "Lack of cohesion of type";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::LackOfCohesionHS;
  typeName.name = "Lack of cohesion of type (Henderson-Sellers variant)";
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
  std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
  const std::string& path_)
{
  _transaction([&, this](){
    typedef odb::query<model::CppAstNodeFilePath> CppAstNodeFilePathQuery;
    typedef odb::result<model::CppAstNodeFilePath> CppAstNodeFilePathResult;
    typedef odb::query<model::CppAstNodeMetricsForPathView> CppAstNodeMetricsForPathViewQuery;
    typedef odb::result<model::CppAstNodeMetricsForPathView> CppAstNodeMetricsForPathViewResult;

    auto nodes = _db->query<model::CppAstNodeMetricsForPathView>(
      CppAstNodeFilePathQuery::LocFile::path.like(path_ + '%'));

    for (const auto& node : nodes)
    {
      CppMetricsAstNodeSingle metric;
      metric.path = node.path;
      metric.type = static_cast<CppAstNodeMetricsType::type>(node.type);
      metric.value = node.value;

      if (_return.count(std::to_string(node.astNodeId)))
      {
        _return[std::to_string(node.astNodeId)].push_back(metric);
      }
      else
      {
        std::vector<CppMetricsAstNodeSingle> metricsList;
        metricsList.push_back(metric);
        _return.insert(std::make_pair(std::to_string(node.astNodeId), metricsList));
      }
    }
  });
}

void CppMetricsServiceHandler::getCppFileMetricsForPath(
  std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
  const std::string& path_)
{
  _transaction([&, this](){
    typedef odb::query<model::CppModuleMetricsForPathView> CppModuleMetricsQuery;
    typedef odb::result<model::CppModuleMetricsForPathView> CppModuleMetricsResult;

    auto files = _db->query<model::CppModuleMetricsForPathView>(
      CppModuleMetricsQuery::File::path.like(path_ + '%'));

    for (const auto& file : files)
    {
      CppMetricsModuleSingle metric;
      metric.path = file.path;
      metric.type = static_cast<CppModuleMetricsType::type>(file.type);
      metric.value = file.value;

      if (_return.count(std::to_string(file.fileId)))
      {
        _return[std::to_string(file.fileId)].push_back(metric);
      }
      else
      {
        std::vector<CppMetricsModuleSingle> metricsList;
        metricsList.push_back(metric);
        _return.insert(std::make_pair(std::to_string(file.fileId), metricsList));
      }
    }
  });
}

} // cppmetrics
} // service
} // cc
