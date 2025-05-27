#include <service/cppmetricsservice.h>
#include <util/dbutil.h>

#include <odb/query.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

namespace cc
{
namespace service
{
namespace cppmetrics
{

typedef odb::query<model::CppAstNodeMetricsForPathView> CppNodeMetricsQuery;
typedef odb::result<model::CppAstNodeMetricsForPathView> CppNodeMetricsResult;
typedef odb::query<model::CppModuleMetricsForPathView> CppModuleMetricsQuery;
typedef odb::result<model::CppModuleMetricsForPathView> CppModuleMetricsResult;

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

  typeName.type = CppAstNodeMetricsType::McCabeFunction;
  typeName.name = "Cyclomatic (McCabe) complexity of function";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::McCabeType;
  typeName.name = "Cyclomatic (McCabe) complexity of type";
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

  typeName.type = CppAstNodeMetricsType::EfferentType;
  typeName.name = "Efferent coupling of type";
  _return.push_back(typeName);

  typeName.type = CppAstNodeMetricsType::AfferentType;
  typeName.name = "Afferent coupling of type";
  _return.push_back(typeName);
}

void CppMetricsServiceHandler::getCppModuleMetricsTypeNames(
  std::vector<CppModuleMetricsTypeName>& _return)
{
  CppModuleMetricsTypeName typeName;

  typeName.type = CppModuleMetricsType::EfferentModule;
  typeName.name = "Efferent coupling of module";
  _return.push_back(typeName);

  typeName.type = CppModuleMetricsType::AfferentModule;
  typeName.name = "Afferent coupling of module";
  _return.push_back(typeName);

  typeName.type = CppModuleMetricsType::RelationalCohesionModule;
  typeName.name = "Relational cohesion of module";
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

void CppMetricsServiceHandler::queryCppAstNodeMetricsForPath(
  std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
  const odb::query<model::CppAstNodeMetricsForPathView>& query_)
{
  _transaction([&, this](){
    auto nodes = _db->query<model::CppAstNodeMetricsForPathView>(query_);

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

void CppMetricsServiceHandler::getCppAstNodeMetricsForPath(
  std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
  const std::string& path_)
{
  queryCppAstNodeMetricsForPath(_return,
    CppNodeMetricsQuery::LocFile::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppAstNodeMetricsForPath(
  std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const std::int32_t pageNumber_)
{
  std::vector<model::CppAstNodeId> paged_nodes = pageMetrics<model::CppAstNodeId, model::CppAstNodeMetricsDistinctView>(
    path_, pageSize_, pageNumber_);

  queryCppAstNodeMetricsForPath(_return,
    CppNodeMetricsQuery::CppAstNodeMetrics::astNodeId.in_range(paged_nodes.begin(), paged_nodes.end()));
}

void CppMetricsServiceHandler::queryCppAstNodeMetricsDetailedForPath(
  std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
  const odb::query<model::CppAstNodeMetricsAndDataForPathView>& query_)
{
  _transaction([&, this](){
    auto nodes = _db->query<model::CppAstNodeMetricsAndDataForPathView>(query_);

    for (const auto& node : nodes)
    {
      auto pair = std::make_pair(static_cast<CppAstNodeMetricsType::type>(node.type), node.value);
      if (_return.count(std::to_string(node.astNodeId)))
      {
        _return[std::to_string(node.astNodeId)].metrics.insert(pair);
      }
      else
      {
        CppMetricsAstNodeDetailed metric;
        std::size_t pos = node.path.find_last_of('/');
        metric.path = node.path.substr(0, pos + 1);
        metric.file = node.path.substr(pos + 1);
        metric.startLine = node.startLine;
        metric.endLine = node.endLine;
        metric.astValue = node.astValue;
        metric.symbolType = cc::model::symbolTypeToString(node.symbolType);
        metric.astType = cc::model::astTypeToString(node.astType);
        metric.metrics.insert(pair);

        _return.insert(std::make_pair(std::to_string(node.astNodeId), metric));
      }
    }
  });
}

void CppMetricsServiceHandler::getCppAstNodeMetricsDetailedForPath(
  std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
  const std::string& path_)
{
  queryCppAstNodeMetricsDetailedForPath(_return,
    CppNodeMetricsQuery::LocFile::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppAstNodeMetricsDetailedForPath(
  std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const std::int32_t pageNumber_)
{
  std::vector<model::CppAstNodeId> paged_nodes = pageMetrics<model::CppAstNodeId, model::CppAstNodeMetricsDistinctView>(
    path_, pageSize_, pageNumber_);

  queryCppAstNodeMetricsDetailedForPath(_return,
    CppNodeMetricsQuery::CppAstNodeMetrics::astNodeId.in_range(paged_nodes.begin(), paged_nodes.end()));
}

void CppMetricsServiceHandler::queryCppFileMetricsForPath(
  std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
  const odb::query<model::CppModuleMetricsForPathView>& query_)
{
  _transaction([&, this](){
    auto files = _db->query<model::CppModuleMetricsForPathView>(query_);

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

void CppMetricsServiceHandler::getCppFileMetricsForPath(
  std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
  const std::string& path_)
{
  queryCppFileMetricsForPath(_return,
    CppModuleMetricsQuery::File::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppFileMetricsForPath(
  std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const std::int32_t pageNumber_)
{
  std::vector<model::FileId> paged_files = pageMetrics<model::FileId, model::CppModuleMetricsDistinctView>(path_, pageSize_, pageNumber_);
  queryCppFileMetricsForPath(_return,
    CppModuleMetricsQuery::CppFileMetrics::file.in_range(paged_files.begin(), paged_files.end()));
}

std::string CppMetricsServiceHandler::getPagingQuery(
  const std::int32_t pageSize_,
  const std::int32_t pageNumber_)
{
  if (pageSize_ <= 0)
  {
    core::InvalidInput ex;
    ex.__set_msg("Invalid page size: " + std::to_string(pageSize_));
    throw ex;
  }

  if (pageNumber_ <= 0)
  {
    core::InvalidInput ex;
    ex.__set_msg("Invalid page number: " + std::to_string(pageNumber_));
    throw ex;
  }

  const std::int32_t offset = (pageNumber_ - 1) * pageSize_;
  return " LIMIT " + std::to_string(pageSize_) + " OFFSET " + std::to_string(offset);
}

} // cppmetrics
} // service
} // cc
