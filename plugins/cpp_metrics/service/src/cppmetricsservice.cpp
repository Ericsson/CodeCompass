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
  std::vector<CppMetricsAstNodeEntry>& result_,
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

      // Try to find existing entry with same astNodeId
      auto it = std::find_if(result_.begin(), result_.end(),
        [&](const CppMetricsAstNodeEntry& entry) {
          return std::stoull(entry.astNodeId) == node.astNodeId;
        });

      if (it != result_.end())
      {
        // Found existing entry → append metric
        it->metrics.push_back(metric);
      }

      else
      {
        // No entry for this astNodeId → create new one
        CppMetricsAstNodeEntry entry;
        entry.astNodeId = std::to_string(node.astNodeId);
        entry.metrics.push_back(metric);
        result_.push_back(std::move(entry));
      }
    }
  });
}

void CppMetricsServiceHandler::getCppAstNodeMetricsForPath(
  std::vector<CppMetricsAstNodeEntry>& _return,
  const std::string& path_)
{
  queryCppAstNodeMetricsForPath(_return,
    CppNodeMetricsQuery::LocFile::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppAstNodeMetricsForPath(
  std::vector<CppMetricsAstNodeEntry>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const core::AstNodeId& previousId_)
{
  std::vector<model::CppAstNodeId> paged_nodes = pageAstNodeMetrics(
    path_, pageSize_, previousId_.empty() ? 0 : std::stoull(previousId_));

  queryCppAstNodeMetricsForPath(_return,
    CppNodeMetricsQuery::CppAstNodeMetrics::astNodeId.in_range(paged_nodes.begin(), paged_nodes.end()));
}

void CppMetricsServiceHandler::queryCppAstNodeMetricsDetailedForPath(
  std::vector<CppMetricsAstNodeDetailedEntry>& result_,
  const odb::query<model::CppAstNodeMetricsAndDataForPathView>& query_)
{
  _transaction([&, this](){
    auto nodes = _db->query<model::CppAstNodeMetricsAndDataForPathView>(query_);

    for (const auto& node : nodes)
    {
      auto pair = std::make_pair(static_cast<CppAstNodeMetricsType::type>(node.type), node.value);

      // Try to find existing entry with same astNodeId
      auto it = std::find_if(result_.begin(), result_.end(),
        [&](const CppMetricsAstNodeDetailedEntry& entry) {
          return std::stoull(entry.astNodeId) == node.astNodeId;
        });

      if (it != result_.end())
      {
        // Found existing entry → append metric
        it->details.metrics.insert(pair);
      }
      else
      {
        // No entry for this astNodeId → create new one
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

        CppMetricsAstNodeDetailedEntry entry;
        entry.astNodeId = std::to_string(node.astNodeId);
        entry.details = metric;

        result_.push_back(std::move(entry));
      }
    }
  });
}

void CppMetricsServiceHandler::getCppAstNodeMetricsDetailedForPath(
  std::vector<CppMetricsAstNodeDetailedEntry>& _return,
  const std::string& path_)
{
  queryCppAstNodeMetricsDetailedForPath(_return,
    CppNodeMetricsQuery::LocFile::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppAstNodeMetricsDetailedForPath(
  std::vector<CppMetricsAstNodeDetailedEntry>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const core::AstNodeId& previousId_)
{
  std::vector<model::CppAstNodeId> paged_nodes = pageAstNodeMetrics(
    path_, pageSize_, previousId_.empty() ? 0 : std::stoull(previousId_));

  queryCppAstNodeMetricsDetailedForPath(_return,
    CppNodeMetricsQuery::CppAstNodeMetrics::astNodeId.in_range(paged_nodes.begin(), paged_nodes.end())
    + ("ORDER BY" + odb::query<model::CppAstNodeMetrics>::astNodeId));
}

void CppMetricsServiceHandler::queryCppFileMetricsForPath(
  std::vector<CppMetricsModuleEntry>& result_,
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

      // Try to find existing entry with same fileId
      auto it = std::find_if(result_.begin(), result_.end(),
        [&](const CppMetricsModuleEntry& entry) {
          return std::stoull(entry.fileId) == file.fileId;
        });

      if (it != result_.end())
      {
        // Found existing entry → append metric
        it->metrics.push_back(metric);
      }

      else
      {
        // No entry for this fileId → create new one
        CppMetricsModuleEntry entry;
        entry.fileId = std::to_string(file.fileId);
        entry.metrics.push_back(metric);
        result_.push_back(std::move(entry));
      }
    }
  });
}

void CppMetricsServiceHandler::getCppFileMetricsForPath(
  std::vector<CppMetricsModuleEntry>& _return,
  const std::string& path_)
{
  queryCppFileMetricsForPath(_return,
    CppModuleMetricsQuery::File::path.like(path_ + '%'));
}

void CppMetricsServiceHandler::getPagedCppFileMetricsForPath(
  std::vector<CppMetricsModuleEntry>& _return,
  const std::string& path_,
  const std::int32_t pageSize_,
  const core::FileId& previousId_)
{
  std::vector<model::FileId> paged_files = pageFileMetrics(
    path_, pageSize_, previousId_.empty() ? 0 : std::stoull(previousId_));

  queryCppFileMetricsForPath(_return,
    CppModuleMetricsQuery::CppFileMetrics::file.in_range(paged_files.begin(), paged_files.end())
    + ("ORDER BY" + odb::query<model::CppFileMetrics>::file));
}

std::string CppMetricsServiceHandler::getLimitQuery(
  const std::int32_t pageSize_)
{
  if (pageSize_ <= 0)
  {
    core::InvalidInput ex;
    ex.__set_msg("Invalid page size: " + std::to_string(pageSize_));
    throw ex;
  }

  return " LIMIT " + std::to_string(pageSize_);
}

std::vector<model::CppAstNodeId> CppMetricsServiceHandler::pageAstNodeMetrics(
  const std::string& path_,
  const std::int32_t pageSize_,
  const model::CppAstNodeId previousId_)
{
  typedef odb::query<model::CppAstNodeMetricsDistinctView> MetricQuery;
  typedef odb::result<model::CppAstNodeMetricsDistinctView> MetricResult;

  return _transaction([&, this](){
    MetricQuery condition = MetricQuery::File::path.like(path_ + '%');
    if (previousId_ != 0) {
      condition = condition && (MetricQuery::CppAstNodeMetrics::astNodeId > previousId_);
    }

    MetricResult paged_nodes = _db->query<model::CppAstNodeMetricsDistinctView>(
      condition +
      ("ORDER BY" + odb::query<model::CppAstNodeMetrics>::astNodeId) +
      getLimitQuery(pageSize_));

    std::vector<model::CppAstNodeId> paged_ids(paged_nodes.size());
    std::transform(paged_nodes.begin(), paged_nodes.end(), paged_ids.begin(),
      [](const model::CppAstNodeMetricsDistinctView& e){
        return e.astNodeId;
      });

    return paged_ids;
  });
}

std::vector<model::FileId> CppMetricsServiceHandler::pageFileMetrics(
  const std::string& path_,
  const std::int32_t pageSize_,
  const model::FileId previousId_)
{
  typedef odb::query<model::CppModuleMetricsDistinctView> MetricQuery;
  typedef odb::result<model::CppModuleMetricsDistinctView> MetricResult;

  return _transaction([&, this](){
    MetricQuery condition = MetricQuery::File::path.like(path_ + '%');
    if (previousId_ != 0) {
      condition = condition && (MetricQuery::CppFileMetrics::file > previousId_);
    }

    MetricResult paged_nodes = _db->query<model::CppModuleMetricsDistinctView>(
      condition +
      ("ORDER BY" + odb::query<model::CppFileMetrics>::file) +
      getLimitQuery(pageSize_));

    std::vector<model::FileId> paged_ids(paged_nodes.size());
    std::transform(paged_nodes.begin(), paged_nodes.end(), paged_ids.begin(),
      [](const model::CppModuleMetricsDistinctView& e){
        return e.fileId;
      });

    return paged_ids;
  });
}

} // cppmetrics
} // service
} // cc
