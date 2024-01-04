#include <gtest/gtest.h>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

using namespace cc;

extern char* dbConnectionString;

class CppMetricsParserTest : public ::testing::Test
{
public:
  CppMetricsParserTest() :
    _db(util::connectDatabase(dbConnectionString)),
    _transaction(_db)
  {}

protected:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
};
