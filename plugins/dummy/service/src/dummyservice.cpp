#include <service/dummyservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace dummy
{

DummyServiceHandler::DummyServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /*datadir_*/,
  const boost::program_options::variables_map& config_)
    : _db(db_), _transaction(db_), _config(config_)
{ 
}

void DummyServiceHandler::getDummyString(std::string& str_)
{
  str_ = _config["dummy-result"].as<std::string>();
}

} // dummy
} // service
} // cc
