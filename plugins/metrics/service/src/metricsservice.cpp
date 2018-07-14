#include <algorithm>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <metricsservice/metricsservice.h>

namespace cc
{
namespace service
{
namespace metrics
{

MetricsServiceHandler::MetricsServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _projectService(db_, datadir_, context_)
{
}

void MetricsServiceHandler::getMetrics(
  std::string& _return,
  const core::FileId& fileId,
  const std::vector<std::string>& fileTypeFilter,
  const MetricsType::type metricsType)
{
  core::FileInfo fileInfo;
  _projectService.getFileInfo(fileInfo, fileId);
  _return = getMetricsFromDir(fileInfo, metricsType, fileTypeFilter);
}

void MetricsServiceHandler::getMetricsTypeNames(
  std::vector<MetricsTypeName>& _return)
{
  MetricsTypeName typeName;

  typeName.type = MetricsType::OriginalLoc;
  typeName.name = "Original lines of code";
  _return.push_back(typeName);

  typeName.type = MetricsType::NonblankLoc;
  typeName.name = "Nonblank lines of code";
  _return.push_back(typeName);

  typeName.type = MetricsType::CodeLoc;
  typeName.name = "Lines of pure code";
  _return.push_back(typeName);
}

std::string MetricsServiceHandler::getMetricsFromDir(
  const core::FileInfo& fileInfo,
  const MetricsType::type metricsType,
  const std::vector<std::string>& fileTypeFilter)
{
  if (fileTypeFilter.empty())
    return "";

  using ptree = boost::property_tree::ptree;
  ptree pt;

  _transaction([&, this](){
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::File> FileQuery;
    typedef odb::result<model::Metrics> MetricsResult;
    typedef odb::query<model::Metrics> MetricsQuery;

    //--- Get files under directory ---//

    FileResult descendants = _db->query<model::File>(
      FileQuery::type.in_range(
        fileTypeFilter.begin(), fileTypeFilter.end()) &&
      FileQuery::path.like(fileInfo.path + '%'));

    //--- Create a vector of these file IDs ---//

    std::vector<model::FileId> descendantFids;
    std::transform(
      descendants.begin(),
      descendants.end(),
      std::back_inserter(descendantFids),
      [](const model::File& file) { return file.id; });

    if (descendantFids.empty())
      return;

    //--- Get metrics for these files ---//

    MetricsResult metrics = _db->query<model::Metrics>(
      MetricsQuery::type == static_cast<model::Metrics::Type>(metricsType) &&
      MetricsQuery::file.in_range(
        descendantFids.begin(), descendantFids.end()));

    //--- Create a property tree ---//

    for (const model::Metrics& metric : metrics)
    {
      core::FileId fileId = std::to_string(metric.file);

      core::FileInfo fileInfo;
      _projectService.getFileInfo(fileInfo, fileId);

      std::string path = fileInfo.path.substr(1);
      pt.put(ptree::path_type{path, '/'}, metric.metric);
    }
  });

  std::stringstream ss;
  boost::property_tree::json_parser::write_json(ss, pt, false);

  return ss.str();
}

}
}
}
