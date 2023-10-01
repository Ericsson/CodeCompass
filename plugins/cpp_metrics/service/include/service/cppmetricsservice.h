#ifndef CC_SERVICE_DUMMY_DUMMYSSERVICE_H
#define CC_SERVICE_DUMMY_DUMMYSSERVICE_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

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

  void getDummyString(std::string& str_);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
};

} // cppmetrics
} // service
} // cc

#endif // CC_SERVICE_DUMMY_CPPMETRICSSSERVICE_H
