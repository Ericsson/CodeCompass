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
    std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
    const std::string& path_) override;

  void getPagedCppAstNodeMetricsForPath(
    std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const std::int32_t pageNumber_) override;

  void getCppAstNodeMetricsDetailedForPath(
    std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
    const std::string& path_) override;

  void getPagedCppAstNodeMetricsDetailedForPath(
    std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const std::int32_t pageNumber_) override;

  void getCppFileMetricsForPath(
    std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
    const std::string& path_) override;

  void getPagedCppFileMetricsForPath(
    std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
    const std::string& path_,
    const std::int32_t pageSize_,
    const std::int32_t pageNumber_) override;

  void getCppAstNodeMetricsTypeNames(
    std::vector<CppAstNodeMetricsTypeName>& _return) override;

  void getCppModuleMetricsTypeNames(
    std::vector<CppModuleMetricsTypeName>& _return) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;

  std::string getPagingQuery(
    const std::int32_t pageSize_,
    const std::int32_t pageNumber_);

  template <typename TID, typename TView>
  std::vector<TID> pageMetrics(
    const std::string& path_,
    const std::int32_t pageSize_,
    const std::int32_t pageNumber_)
  {
    return _transaction([&, this](){
      odb::result<TView> paged_nodes = _db->query<TView>(
        odb::query<TView>::File::path.like(path_ + '%') + getPagingQuery(pageSize_, pageNumber_));

      std::vector<TID> paged_ids(paged_nodes.size());
      std::transform(paged_nodes.begin(), paged_nodes.end(), paged_ids.begin(),
        [](const TView& e){
            if constexpr (std::is_same<TView, model::CppAstNodeMetricsDistinctView>::value) {
              return e.astNodeId;
            } else if constexpr (std::is_same<TView, model::CppModuleMetricsDistinctView>::value) {
              return e.fileId;
            }
        });

      return paged_ids;
    });
  }

  void queryCppAstNodeMetricsForPath(
    std::map<core::AstNodeId, std::vector<CppMetricsAstNodeSingle>>& _return,
    const odb::query<model::CppAstNodeMetricsForPathView>& query_);

  void queryCppAstNodeMetricsDetailedForPath(
    std::map<core::AstNodeId, CppMetricsAstNodeDetailed>& _return,
    const odb::query<model::CppAstNodeMetricsAndDataForPathView>& query_);

  void queryCppFileMetricsForPath(
    std::map<core::FileId, std::vector<CppMetricsModuleSingle>>& _return,
    const odb::query<model::CppModuleMetricsForPathView>& query_);
};

} // cppmetrics
} // service
} // cc

#endif // CC_SERVICE_CPPMETRICSSERVICE_H
