#ifndef CC_SERVICE_CPPMETRICSSERVICE_H
#define CC_SERVICE_CPPMETRICSSERVICE_H

#include <memory>
#include <vector>
#include <map>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <CppMetricsService.h>

namespace cc
{
namespace service
{
namespace cppmetrics
{

class CppMetricsServiceHandler : virtual public CppMetricsServiceIf
{
public:
  CppMetricsServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  double getSingleCppMetricForAstNode(
    const core::AstNodeId& astNodeId_,
    CppAstNodeMetricsType::type metric_) override;

  void getCppMetricsForAstNode(
    std::vector<CppMetricsAstNodeSingle>& _return,
    const core::AstNodeId& astNodeId_) override;

  void getCppMetricsForModule(
    std::vector<CppMetricsModuleSingle>& _return,
    const core::FileId& fileId_) override;

  void getCppAstNodeMetricsForPath(
    std::vector<CppMetricsAstNodeEntry>& _return,
    const std::string& path_) override;

  void getPagedCppAstNodeMetricsForPath(
    std::vector<CppMetricsAstNodeEntry>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const core::AstNodeId& previousId_) override;

  void getCppAstNodeMetricsDetailedForPath(
    std::vector<CppMetricsAstNodeDetailedEntry>& _return,
    const std::string& path_) override;

  void getPagedCppAstNodeMetricsDetailedForPath(
    std::vector<CppMetricsAstNodeDetailedEntry>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const core::AstNodeId& previousId_) override;

  void getCppFileMetricsForPath(
    std::vector<CppMetricsModuleEntry>& _return,
    const std::string& path_) override;

  void getPagedCppFileMetricsForPath(
    std::vector<CppMetricsModuleEntry>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const core::FileId& previousId_) override;

  void getCppAstNodeMetricsTypeNames(
    std::vector<CppAstNodeMetricsTypeName>& _return) override;

  void getCppModuleMetricsTypeNames(
    std::vector<CppModuleMetricsTypeName>& _return) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;

  std::string getLimitQuery(const std::int32_t pageSize_);

  std::vector<model::CppAstNodeId> pageAstNodeMetrics(
    const std::string& path_,
    const std::int32_t pageSize_,
    const model::CppAstNodeId previousId_);

  std::vector<model::FileId> pageFileMetrics(
    const std::string& path_,
    const std::int32_t pageSize_,
    const model::FileId previousId_);

  void queryCppAstNodeMetricsForPath(
    std::vector<CppMetricsAstNodeEntry>& result_,
    const odb::query<model::CppAstNodeMetricsForPathView>& query_);

  void queryCppAstNodeMetricsDetailedForPath(
    std::vector<CppMetricsAstNodeDetailedEntry>& result_,
    const odb::query<model::CppAstNodeMetricsAndDataForPathView>& query_);

  void queryCppFileMetricsForPath(
    std::vector<CppMetricsModuleEntry>& result_,
    const odb::query<model::CppModuleMetricsForPathView>& query_);
};

} // cppmetrics
} // service
} // cc

#endif // CC_SERVICE_CPPMETRICSSERVICE_H
