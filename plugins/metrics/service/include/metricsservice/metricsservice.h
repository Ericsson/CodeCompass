#ifndef CC_SERVICE_METRICSSERVICE_H
#define CC_SERVICE_METRICSSERVICE_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/metrics.h>
#include <model/metrics-odb.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/odbtransaction.h>

#include <MetricsService.h>

namespace cc
{
namespace service
{
namespace metrics
{

class MetricsServiceHandler : virtual public MetricsServiceIf
{
public:
  MetricsServiceHandler(
    std::shared_ptr<odb::database> db_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());

  void getMetrics(
    std::string& _return,
    const core::FileId& fileId,
    const std::vector<std::string>& fileTypeFilter,
    const MetricsType::type metricsType) override;

  void getMetricsTypeNames(
    std::vector<MetricsTypeName>& _return) override;

private:
  typedef odb::result<model::Metrics> MetricsResult;
  typedef odb::query<model::Metrics> MetricsQuery;
  
  std::string getMetricsFromDir(
    const core::FileInfo& fileInfo,
    const MetricsType::type metricsType,
    const std::vector<std::string>& fileTypeFilter);

  core::FileInfo getFileInfo(const core::FileId& fileId);
  core::FileInfo makeFileInfo(const model::File& f);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
};

} // metrics
} // service
} // cc

#endif
