#ifndef CC_SERVICE_CODECHECKERPROXY_H
#define CC_SERVICE_CODECHECKERPROXY_H

#include <memory>

#include <boost/program_options/variables_map.hpp>

#include <util/clientwrapper.h>

#include <codeCheckerDBAccess.h>

namespace odb {
  class database;
}

namespace cc { namespace util {
  template <typename Client> class ClientWrapper;
} }

namespace cc
{
namespace service
{
namespace codechecker
{

class CodeCheckerProxyServiceHandler : virtual public codeCheckerDBAccessIf {
public:

  CodeCheckerProxyServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());

private:
  virtual void getRunData(RunDataList& _return) override;
  virtual void getRunResults(ReportDataList& _return, const int64_t runId, const int64_t limit, const int64_t offset, const std::vector<SortMode> & sortType, const ReportFilterList& reportFilters) override;
  virtual int64_t getRunResultCount(const int64_t runId, const ReportFilterList& reportFilters) override;
  virtual void getReportDetails(ReportDetails& _return, const int64_t reportId) override;
  virtual void getSourceFileData(SourceFileData& _return, const int64_t fileId, const bool fileContent) override;
  virtual int64_t getFileId(const int64_t runId, const std::string& path) override;
  virtual void getCheckersListForFile(std::map<std::string, int64_t> & _return, const int64_t fileId) override;
  virtual bool suppressBug(const std::vector<int64_t> & runIds, const int64_t reportId, const std::string& comment) override;
  virtual bool unSuppressBug(const std::vector<int64_t> & runIds, const int64_t reportId) override;
  virtual void getCheckerDoc(std::string& _return, const std::string& checkerId) override;
  virtual void getNewResults(ReportDataList& _return, const int64_t base_run_id, const int64_t new_run_id, const int64_t limit, const int64_t offset, const std::vector<SortMode> & sortType, const ReportFilterList& reportFilters) override;
  virtual void getResolvedResults(ReportDataList& _return, const int64_t base_run_id, const int64_t new_run_id, const int64_t limit, const int64_t offset, const std::vector<SortMode> & sortType, const ReportFilterList& reportFilters) override;
  virtual void getUnresolvedResults(ReportDataList& _return, const int64_t base_run_id, const int64_t new_run_id, const int64_t limit, const int64_t offset, const std::vector<SortMode> & sortType, const ReportFilterList& reportFilters) override;
  virtual void getCheckerConfigs( ::CheckerConfigList& _return, const int64_t runId) override;
  virtual void getSkipPaths(SkipPathDataList& _return, const int64_t runId) override;
  virtual void getBuildActions(std::vector<std::string> & _return, const int64_t reportId) override;
  virtual void getRunResultTypes(ReportDataTypeCountList& _return, const int64_t runId, const ReportFilterList& reportFilters) override;
  virtual void getAPIVersion(std::string& _return) override;
  virtual bool removeRunResults(const std::vector<int64_t> & runIds) override;

private:
  using CodeCheckerClient = util::ClientWrapper<codeCheckerDBAccessClient>;

  int64_t _runId = 0;

  std::string _host;
  std::string _path;
  int _port;
};

} // codechecker
} // service
} // cc


#endif // CC_SERVICE_CODECHECKERPROXY_H


