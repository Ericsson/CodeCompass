#include <algorithm>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <util/dbutil.h>

#include <metricsservice/metricsservice.h>
#include <webserver/servicenotavailexception.h>

namespace cc
{
namespace service
{
namespace metrics
{

MetricsServiceHandler::MetricsServiceHandler(
  std::shared_ptr<odb::database> db_,
  const boost::program_options::variables_map& config_)
  : _db(db_), _transaction(db_), _config(config_)
{
}

void MetricsServiceHandler::getMetrics(
  std::string& _return,
  const core::FileId& fileId,
  const std::vector<std::string>& fileTypeFilter,
  const MetricsType::type metricsType)
{
  core::FileInfo fileInfo = getFileInfo(fileId);
  _return = getMetricsFromDir(fileInfo, metricsType, fileTypeFilter);
}

void MetricsServiceHandler::getMetricsTypeNames(
  std::vector<MetricsTypeName>& _return)
{
  MetricsTypeName typeName;

  typeName.type = MetricsType::McCabe;
  typeName.name = "McCabe";
  _return.push_back(typeName);

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
  if(fileTypeFilter.empty())
    return "";

  using ptree = boost::property_tree::ptree;
  ptree pt;

  _transaction([&, this](){
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::File> FileQuery;

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

    if(descendantFids.empty())
      return "";

    //--- Get metrics for these files ---//

    MetricsResult metrics = _db->query<model::Metrics>(
      MetricsQuery::type == static_cast<model::Metrics::Type>(metricsType) &&
      MetricsQuery::file.in_range(
        descendantFids.begin(), descendantFids.end()));

    //--- Create a property tree ---//

    for (const model::Metrics& metric : metrics)
    {
      core::FileId fileId = std::to_string(metric.file);

      std::string path = getFileInfo(fileId).path.substr(1);
      pt.put(ptree::path_type{path, '/'}, metric.metric);
    }
  });

  std::stringstream ss;
  boost::property_tree::json_parser::write_json(ss, pt, false);
  
  return ss.str();
}

core::FileInfo MetricsServiceHandler::getFileInfo(const core::FileId& fileId)
{
  return _transaction([&, this](){
    try
    {
      model::File f;
      
      if (!_db->find<model::File>(std::stoull(fileId), f))
      {
        core::InvalidId ex;
        ex.__set_fid(fileId);
        ex.__set_msg("Invalid file ID");
        throw ex;
      }
      
      return makeFileInfo(f);
    }
    catch (odb::exception &odbex)
    {
      BOOST_LOG_TRIVIAL(warning)
        << "datasource exception occured: " << odbex.what();
    }
  });
}

// TODO: This function is copy-pasted from another service. This should be
// written only once.
core::FileInfo MetricsServiceHandler::makeFileInfo(const model::File& f)
{    
  core::FileInfo fileInfo;

  fileInfo.id = std::to_string(f.id);
  fileInfo.name = f.filename;
  fileInfo.path = f.path;
  fileInfo.isDirectory = f.type == model::File::DIRECTORY_TYPE;
  fileInfo.type = static_cast<std::string>(f.type);

  if (f.parent)
    fileInfo.parent = std::to_string(f.parent.object_id());

  switch (f.parseStatus)
  {
  case model::File::PSNone:
    fileInfo.parseStatus = core::FileParseStatus::Nothing;
    break;
  case model::File::PSPartiallyParsed:
    fileInfo.parseStatus = core::FileParseStatus::PartiallyParsed;
    break;
  case model::File::PSFullyParsed:
    fileInfo.parseStatus = core::FileParseStatus::FullyParsed;
    break;
  default:
    fileInfo.parseStatus = core::FileParseStatus::Nothing;
    break;
  }

  if (fileInfo.parseStatus == core::FileParseStatus::Nothing &&
      f.inSearchIndex)
    fileInfo.parseStatus = core::FileParseStatus::OnlyInSearchIndex;
  
  return fileInfo;
}

}
}
}
