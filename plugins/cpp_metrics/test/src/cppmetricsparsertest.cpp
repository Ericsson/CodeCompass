#include <math.h>
#include <gtest/gtest.h>

// Same as EXPECT_NEAR, but NANs are also considered equal.
#define EXPECT_EQF(val1, val2, abs_error) { \
  const auto v1 = val1; const auto v2 = val2; \
  const bool n1 = isnan(v1); const bool n2 = isnan(v2); \
  EXPECT_EQ(n1, n2); \
  if (!n1 && !n2) EXPECT_NEAR(v1, v2, abs_error); \
}

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>

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
  typedef model::CppAstNodeMetrics::Type Type;

  bool queryRecordMetric(
    const std::string& qualType_,
    Type metricType_,
    double& result_);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
};

bool CppMetricsParserTest::queryRecordMetric(
  const std::string& qualType_,
  Type metricType_,
  double& result_)
{
  typedef odb::query<model::CppRecordMetricsView>::query_columns QEntry;
  const auto& QEntryQualName = QEntry::CppRecord::qualifiedName;
  const auto& QEntryType = QEntry::CppAstNodeMetrics::type;

  model::CppRecordMetricsView entry;
  if (_db->query_one<model::CppRecordMetricsView>(
      QEntryQualName == qualType_ && QEntryType == metricType_, entry))
  {
    result_ = entry.value;
    return true;
  }
  else return false;
}

// McCabe

typedef std::pair<std::string, unsigned int> McCabeParam;

class ParameterizedMcCabeTest
  : public CppMetricsParserTest,
    public ::testing::WithParamInterface<McCabeParam>
{};

std::vector<McCabeParam> paramMcCabe = {
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
  ::testing::ValuesIn(paramMcCabe)
);

// Lack of Cohesion

struct LackOfCohesionParam
{
  std::string typeName;
  double loc;
  double locHS;
};

class ParameterizedLackOfCohesionTest
  : public CppMetricsParserTest,
    public ::testing::WithParamInterface<LackOfCohesionParam>
{};

constexpr double C1_3 = 1.0 / 3.0;
std::vector<LackOfCohesionParam> paramLackOfCohesion = {
  {"trivial_0_0", 0, 0},
  {"trivial_1_0", 0, 0},
  {"trivial_2_0", 0, 0},
  {"trivial_3_0", 0, 0},
  {"trivial_0_1", 0, 0},
  {"trivial_0_2", 0, 0},
  {"trivial_0_3", 0, 0},
  {"fully_cohesive_read", 0, NAN},
  {"fully_cohesive_write", 0, NAN},
  {"fully_cohesive_read_write", 0, NAN},
  {"fully_cohesive_2_1", 0, NAN},
  {"fully_cohesive_1_2", 0, 0},
  {"fully_cohesive_2_2", 0, 0},
  {"fully_cohesive_3_3", 0, 0},
  {"fully_incohesive_1_1", 1, NAN},
  {"fully_incohesive_1_2", 1, 2},
  {"fully_incohesive_2_1", 1, NAN},
  {"fully_incohesive_2_2", 1, 2},
  {"partially_cohesive_1_2", 0.5, 1},
  {"partially_cohesive_2_1", 0.5, NAN},
  {"partially_cohesive_2_2_A", 0.75, 1.5},
  {"partially_cohesive_2_2_B", 0.5, 1},
  {"partially_cohesive_2_2_C", 0.25, 0.5},
  {"same_partial_coh_A", C1_3, 0.5},
  {"same_partial_coh_B", C1_3, 0.5},
  {"same_partial_coh_C", C1_3, 0.5},
};

TEST_P(ParameterizedLackOfCohesionTest, LackOfCohesionTest) {
  _transaction([&, this]() {
    const std::string& qualType = GetParam().typeName;
    double loc, locHS;
    ASSERT_TRUE(queryRecordMetric(qualType, Type::LACK_OF_COHESION, loc));
    ASSERT_TRUE(queryRecordMetric(qualType, Type::LACK_OF_COHESION_HS, locHS));

    constexpr double tolerance = 1e-8;
    EXPECT_EQF(GetParam().loc, loc, tolerance);
    EXPECT_EQF(GetParam().locHS, locHS, tolerance);
  });
}

INSTANTIATE_TEST_SUITE_P(
  ParameterizedLackOfCohesionTestSuite,
  ParameterizedLackOfCohesionTest,
  ::testing::ValuesIn(paramLackOfCohesion)
);
