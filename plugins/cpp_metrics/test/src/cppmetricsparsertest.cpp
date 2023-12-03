#include <gtest/gtest.h>

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>

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

class ParameterizedMcCabeTest
  : public CppMetricsParserTest,
    public ::testing::WithParamInterface<std::pair<std::string, unsigned int>>
{};

std::vector<std::pair<std::string, unsigned int>> parameters = {
  {"conditionals", 8},
  {"loops1", 6},
  {"loops2", 6},
  {"loops3", 5},
  {"switchcase", 7},
  {"fragile", 1},
  {"trycatch", 3},
  {"MyClass::MyClass", 2},
  {"MyClass::~MyClass", 2},
  {"MyClass::method1", 8}
};

TEST_P(ParameterizedMcCabeTest, McCabeTest) {
  _transaction([&, this]() {
    model::CppFunction func = _db->query_value<model::CppFunction>(
      odb::query<model::CppFunction>::qualifiedName == GetParam().first);

    EXPECT_EQ(GetParam().second, func.mccabe);
  });
}

INSTANTIATE_TEST_SUITE_P(
  ParameterizedMcCabeTestSuite,
  ParameterizedMcCabeTest,
  ::testing::ValuesIn(parameters)
  );