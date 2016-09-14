#include "codecheckerproxy.h"

#include <util/streamlog.h>
#include <plugin/servicenotavailexception.h>
#include <util/clientwrapper.h>

namespace cc
{
namespace service
{
namespace codechecker
{

namespace
{

/**
 * Parses a "host:port" string into host and port. If no colon then the default
 * port will be used given as the third parameter.
 */
void fillHostPort(
  const std::string& urlPart_, std::string& host_, int& port_, int defaultPort_)
{
  size_t colonPos = urlPart_.find(':');
  
  if (colonPos == std::string::npos)
  {
    host_ = urlPart_;
    port_ = defaultPort_;
  }
  else
  {
    host_ = urlPart_.substr(0, colonPos);
    std::string portString = urlPart_.substr(colonPos + 1);
    port_ = std::stoi(portString);
    
    // Easier error handling as 0 is invalid anyway.
    if (0 == port_)
    {
      std::string errorMsg
        = "CodeCheckerProxy - invalid port in URL: " + portString;
      
      SLog(util::ERROR) << errorMsg;
      throw std::runtime_error(errorMsg);
    }
  }
}

}

CodeCheckerProxyServiceHandler::CodeCheckerProxyServiceHandler(
  std::shared_ptr<odb::database> db_,
  const boost::program_options::variables_map& config_)
{
  const int httpDefaultPort = 80;
  
  //--- Load URL from configuration ---//
  
  auto configIt = config_.find("codecheckerurl");
  
  if (configIt == config_.end())
    throw ServiceNotAvailException(
      "CodeChecker service is not available because CodeChecker server URL is "
      "not provided");
  
  std::string url = configIt->second.as<std::string>();
  
  //--- Parse URL ---//
  
  if (url.substr(0, 7) != "http://")
  {
    std::string errorMsg = "CodeCheckerProxy supports http URLs only. Please "
      "set codecheckerurl parameter accordingly.";

    SLog(util::ERROR) << errorMsg;
    throw std::runtime_error(errorMsg);
  }

  url = url.substr(7);
  std::size_t slashPos = url.find('/');
  
  fillHostPort(url.substr(0, slashPos), _host, _port, httpDefaultPort);
  _path = slashPos == std::string::npos ? "/" : url.substr(slashPos);
  
  // Rebuild the URL to see in logs if parsed incorrectly.
  SLog(util::DEBUG)
    << "CodeChecker enabled, server url is http://"
    << _host << ":" << _port << _path;
  
  try {
    CodeCheckerClient(_host, _port, _path);
  } catch (const apache::thrift::transport::TTransportException& ex) {
    throw ServiceNotAvailException("CodeChecker service is not available");
  }
  
  //--- Get CodeChecker run id ---//
  
  configIt = config_.find("codecheckerrunid");
  
  if (configIt != config_.end())
  {
    std::string sRunId = configIt->second.as<std::string>();
    try {
      _runId = std::stoll(sRunId);
    } catch (std::invalid_argument) {
      SLog(util::ERROR)
        << "codecheckerrunid cannot be parsed as a number: " + sRunId;
      throw;
    } catch (std::out_of_range) {
      SLog(util::ERROR)
        << "codecheckerrunid cannot be parsed as a number: " + sRunId;
      throw;
    }
  }
  
  //--- Get CodeChecker run id by run name ---//
  
  configIt = config_.find("codecheckerrunname");
  
  if (configIt != config_.end())
  {
    std::string runName = configIt->second.as<std::string>();

    RunDataList runList;
    CodeCheckerClient(_host, _port, _path).client()->getRunData(runList);

    std::vector<RunData>::const_iterator it = std::find_if(
      runList.begin(), runList.end(), [&runName](const RunData& rd) {
        return rd.name == runName;
      });

    if (it == runList.end())
    {
      std::string errorMsg
        = "No CodeChecker run found with this name: " + runName;

      SLog(util::ERROR) << errorMsg;
      throw ServiceNotAvailException(errorMsg);
    }
    else
      _runId = it->runId;
  }
}

void CodeCheckerProxyServiceHandler::getRunData(RunDataList& _return)
{
  CodeCheckerClient(_host, _port, _path).client()->getRunData(_return);
}

void CodeCheckerProxyServiceHandler::getRunResults(
  ReportDataList& _return,
  const int64_t runId,
  const int64_t limit,
  const int64_t offset,
  const std::vector<SortMode>& sortType,
  const ReportFilterList& reportFilters)
{
  CodeCheckerClient(_host, _port, _path).client()->getRunResults(
    _return, _runId, limit, offset, sortType, reportFilters);
}

int64_t CodeCheckerProxyServiceHandler::getRunResultCount(
  const int64_t runId, const ReportFilterList& reportFilters)
{
  return CodeCheckerClient(_host, _port, _path).client()->getRunResultCount(
    _runId, reportFilters);
}

void CodeCheckerProxyServiceHandler::getReportDetails(
  ReportDetails& _return, const int64_t reportId)
{
  CodeCheckerClient(_host, _port, _path).client()->getReportDetails(
    _return, reportId);
}

void CodeCheckerProxyServiceHandler::getSourceFileData(
  SourceFileData& _return, const int64_t fileId, const bool fileContent)
{
  CodeCheckerClient(_host, _port, _path).client()->getSourceFileData(
    _return, fileId, fileContent);
}

int64_t CodeCheckerProxyServiceHandler::getFileId(
  const int64_t runId, const std::string& path)
{
  return CodeCheckerClient(_host, _port, _path).client()->getFileId(
    _runId, path);
}

void CodeCheckerProxyServiceHandler::getCheckersListForFile(
  std::map<std::string, int64_t>& _return, const int64_t fileId)
{
  CodeCheckerClient(_host, _port, _path).client()->getCheckersListForFile(
    _return, fileId);
}

bool CodeCheckerProxyServiceHandler::suppressBug(
  const std::vector<int64_t>& runIds,
  const int64_t reportId,
  const std::string& comment)
{
  return CodeCheckerClient(_host, _port, _path).client()->suppressBug(
    {_runId}, reportId, comment);
}

bool CodeCheckerProxyServiceHandler::unSuppressBug(
  const std::vector<int64_t>& runIds, const int64_t reportId)
{
  throw std::runtime_error("Not yet implemented");
}

void CodeCheckerProxyServiceHandler::getCheckerDoc(
  std::string& _return, const std::string& checkerId)
{
  CodeCheckerClient(_host, _port, _path).client()->getCheckerDoc(
    _return, checkerId);
}

void CodeCheckerProxyServiceHandler::getNewResults(
  ReportDataList& _return,
  const int64_t base_run_id,
  const int64_t new_run_id,
  const int64_t limit,
  const int64_t offset,
  const std::vector<SortMode>& sortType,
  const ReportFilterList& reportFilters)
{
  throw std::runtime_error("Not yet implemented");
}

void CodeCheckerProxyServiceHandler::getResolvedResults(
  ReportDataList& _return,
  const int64_t base_run_id,
  const int64_t new_run_id,
  const int64_t limit,
  const int64_t offset,
  const std::vector<SortMode>& sortType,
  const ReportFilterList& reportFilters)
{
  throw std::runtime_error("Not yet implemented");
}

void CodeCheckerProxyServiceHandler::getUnresolvedResults(
  ReportDataList& _return,
  const int64_t base_run_id,
  const int64_t new_run_id,
  const int64_t limit,
  const int64_t offset,
  const std::vector<SortMode>& sortType,
  const ReportFilterList& reportFilters)
{
  throw std::runtime_error("Not yet implemented");
}

void CodeCheckerProxyServiceHandler::getCheckerConfigs(
  ::CheckerConfigList& _return, const int64_t runId)
{
  CodeCheckerClient(_host, _port, _path).client()->getCheckerConfigs(
    _return, _runId);
}

void CodeCheckerProxyServiceHandler::getSkipPaths(
  SkipPathDataList& _return, const int64_t runId)
{
  throw std::runtime_error("Not yet implemented");
}

void CodeCheckerProxyServiceHandler::getBuildActions(
  std::vector<std::string>& _return, const int64_t reportId)
{
  CodeCheckerClient(_host, _port, _path).client()->getBuildActions(
    _return, reportId);
}

void CodeCheckerProxyServiceHandler::getRunResultTypes(
  ReportDataTypeCountList& _return,
  const int64_t runId,
  const ReportFilterList& reportFilters)
{
  CodeCheckerClient(_host, _port, _path).client()->getRunResultTypes(
    _return, _runId, reportFilters);
}

void CodeCheckerProxyServiceHandler::getAPIVersion(std::string& _return)
{
  CodeCheckerClient(_host, _port, _path).client()->getAPIVersion(_return);
}

bool CodeCheckerProxyServiceHandler::removeRunResults(
  const std::vector<int64_t> & runIds)
{
  throw std::runtime_error("Not yet implemented");
}

} // codechecker
} // service
} // cc
