#include <algorithm>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "metricsservice.h"
#include <plugin/servicenotavailexception.h>

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
  _transaction([&, this](){
    try
    {
      // TODO: Better solution for checking table existence or emptyness?
      _db->query_value<model::MetricsCount>();
    }
    catch (const odb::exception& ex)
    {
      throw ServiceNotAvailException("MetricsService is not available");
    }
  });
  
  // Make CodeChecker metrics available if CodeChecker server if available,
  // i.e. "codecheckerurl" is given in config_ parameter.
  
  auto configIt = config_.find("codecheckerurl");
  if (configIt != config_.end())
  {
    // url variable now contains the CodeChecker url without "http://" at the
    // begining.
    std::string url = configIt->second.as<std::string>().substr(7);
    std::size_t slashPos = url.find('/');
    
    _codeCheckerPath
      = slashPos == std::string::npos
      ? "/"
      : url.substr(slashPos);
    
    url = url.substr(0, slashPos);
    slashPos = url.find(':');
    
    _codeCheckerHost = url.substr(0, slashPos);
    _codeCheckerPort
      = slashPos == std::string::npos
      ? 80 // Use 80 as default port
      : std::stoi(url.substr(slashPos + 1));
  }
}

void MetricsServiceHandler::getMetrics(
  std::string& _return,
  const core::FileId& fileId,
  const std::vector<core::FileType::type>& fileTypeFilter,
  const MetricsType::type metricsType)
{
  core::FileInfo fileInfo = getFileInfo(fileId);
  _return = metricsType == MetricsType::CodeChecker
    ? getCheckerMetrics(fileInfo)
    : getMetricsFromDir(fileInfo, metricsType);
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
  
  bool hasCodeChecker = true;
  
  try {
    _cw = std::make_shared<CodeCheckerClient>(
      _codeCheckerHost, _codeCheckerPort, _codeCheckerPath);
  } catch (...) {
    hasCodeChecker = false;
  }
  
  if (hasCodeChecker)
  {
    typeName.type = MetricsType::CodeChecker;
    typeName.name = "CodeChecker bugs";
    _return.push_back(typeName);
  }
}

std::string MetricsServiceHandler::getMetricsFromDir(
  const core::FileInfo& fileInfo,
  const MetricsType::type metricsType)
{
  using ptree = boost::property_tree::ptree;
  ptree pt;
  
  _transaction([&, this](){
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::File> FileQuery;

    //--- Get files under directory ---//
    
    FileResult descendants = _db->query<model::File>(
      FileQuery::type != model::File::Directory &&
      FileQuery::path.like(fileInfo.path + '%'));

    //--- Create a vector of these file IDs ---//
    
    std::vector<model::FileId> descendantFids;
    std::transform(
      descendants.begin(),
      descendants.end(),
      std::back_inserter(descendantFids),
      [](const model::File& file) { return file.id; });

    //--- Get metrics for these files ---//
      
    MetricsResult metrics = _db->query<model::Metrics>(
      MetricsQuery::type == static_cast<model::Metrics::Type>(metricsType) &&
      MetricsQuery::file.in_range(
        descendantFids.begin(), descendantFids.end()));
    
    //--- Create a property tree ---//
    
    for (const model::Metrics& metric : metrics)
    {
      core::FileId fileId;
      fileId.__set_fid(std::to_string(metric.file));
      
      std::string path = getFileInfo(fileId).path.substr(1);
      pt.put(ptree::path_type{path, '/'}, metric.metric);
    }
  });
  
  std::stringstream ss;
  boost::property_tree::json_parser::write_json(ss, pt, false);
  
  return ss.str();
}

std::string MetricsServiceHandler::getCheckerMetrics(
  const core::FileInfo& fileInfo)
{
  //--- Query options ---//
  
  codechecker::ReportFilterList filterList;
  filterList.reserve(3);
  
  codechecker::ReportFilter filter;
  filter.__set_filepath(fileInfo.path + '*');
  
  filter.__set_severity(Severity::CRITICAL);
  filterList.push_back(filter);
  filter.__set_severity(Severity::HIGH);
  filterList.push_back(filter);
  filter.__set_severity(Severity::MEDIUM);
  filterList.push_back(filter);
  
  util::ClientWrapper<codechecker::codeCheckerDBAccessClient> cw(
    _codeCheckerHost, _codeCheckerPort, _codeCheckerPath);
  
  codechecker::SortMode sortMode;
  sortMode.type = codechecker::SortType::FILENAME;
  
  //--- Do the query ---//
  
  codechecker::ReportDataList result;
  cw.client()->getRunResults(result, 1, 1000, 0, {sortMode}, {filter});
  
  //--- Process result ---//
  
  using ptree = boost::property_tree::ptree;
  ptree pt;
  
  std::string filename = result.empty() ? "" : result[0].checkedFile;
  int counter = 0;
  
  for (const codechecker::ReportData& reportData : result)
    if (reportData.checkedFile == filename)
      ++counter;
    else
    {
      pt.put(ptree::path_type{filename.substr(1), '/'}, counter);
      filename = reportData.checkedFile;
      counter = 1;
    }
  
  if (!filename.empty())
    pt.put(ptree::path_type{filename.substr(1), '/'}, counter);
    
  //--- Return result ---//
  
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
      
      if (!_db->find<model::File>(std::stoull(fileId.fid), f))
      {
        core::InvalidId ex;
        ex.fid = fileId.fid;
        ex.what = "invalid file ID";
        throw ex;
      }
      
      return makeFileInfo(f);
    }
    catch (odb::exception &odbex)
    {
      SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
      
      core::DatasourceError ex;
      ex.what = odbex.what();
      throw ex;
    }       
  });
}

// TODO: This function is copy-pasted from another service. This should be
// written only once.
core::FileInfo MetricsServiceHandler::makeFileInfo(const model::File& f)
{    
  core::FileInfo fileInfo;

  fileInfo.file.fid = std::to_string(f.id);
  fileInfo.name = f.filename;
  fileInfo.path = f.path;
  fileInfo.isDirectory = f.type == model::File::Type::Directory;
  fileInfo.type = static_cast<core::FileType::type>(f.type);
  
  if (f.parent)
    fileInfo.parent.fid = std::to_string(f.parent.object_id());
  
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
