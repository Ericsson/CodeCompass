#ifndef CC_SERVICE_DUMMY_DUMMYSSERVICE_H
#define CC_SERVICE_DUMMY_DUMMYSSERVICE_H

#include <memory>
#include <vector>

#include <odb/database.hxx>
#include <util/odbtransaction.h>

#include <boost/program_options/variables_map.hpp>

#include <DummyService.h>

namespace cc
{
namespace service
{
namespace dummy
{

class DummyServiceHandler : virtual public DummyServiceIf
{
public:
  DummyServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());

  void getDummyString(std::string& str_);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
};

} // dummy
} // service
} // cc

#endif // CC_SERVICE_DUMMY_DUMMYSSERVICE_H
