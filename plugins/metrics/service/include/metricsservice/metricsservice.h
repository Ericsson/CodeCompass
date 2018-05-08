#ifndef CC_SERVICE_METRICS_H
#define CC_SERVICE_METRICS_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/metrics.h>
#include <model/metrics-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <projectservice/projectservice.h>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

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
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getMetrics(
    std::string& _return,
    const core::FileId& fileId,
    const std::vector<std::string>& fileTypeFilter,
    const MetricsType::type metricsType) override;

  void getMetricsTypeNames(
    std::vector<MetricsTypeName>& _return) override;

private:
  std::string getMetricsFromDir(
    const core::FileInfo& fileInfo,
    const MetricsType::type metricsType,
    const std::vector<std::string>& fileTypeFilter);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  core::ProjectServiceHandler _projectService;
};

} // metrics
} // service
} // cc

#endif
