#ifndef CC_SERVICE_CPPMETRICSSERVICE_H
#define CC_SERVICE_CPPMETRICSSERVICE_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/cppastnodemetrics.h>
//#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

//#include <projectservice/projectservice.h>

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

  //double getCppMetricsForAstNode(
    //const ::cc::service::core::AstNodeId& astNodeId_,
    //const CppMetricsType::type metrics_) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
};

} // cppmetrics
} // service
} // cc

#endif // CC_SERVICE_CPPMETRICSSERVICE_H
