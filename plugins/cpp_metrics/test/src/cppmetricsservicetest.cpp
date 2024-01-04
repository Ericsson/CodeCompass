#include <gtest/gtest.h>

#include <service/cppmetricsservice.h>

#include <util/dbutil.h>

using namespace cc;
using namespace service::cppmetrics;

extern char* dbConnectionString;

class CppMetricsServiceTest : public ::testing::Test
{
public:
  CppMetricsServiceTest() :
    _db(util::connectDatabase(dbConnectionString)),
    _transaction(_db),
    _cppservice(new CppMetricsServiceHandler(
      _db,
      std::make_shared<std::string>(""),
      webserver::ServerContext(std::string(),
        boost::program_options::variables_map())))
  {

  }

protected:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::shared_ptr<CppMetricsServiceHandler> _cppservice;
};