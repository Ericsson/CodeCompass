#include <math.h>
#include <gtest/gtest.h>

// Same as EXPECT_NEAR, but NANs are also considered equal.
#define EXPECT_EQF(val1, val2, abs_error) { \
  const auto v1 = val1; const auto v2 = val2; \
  const bool n1 = isnan(v1); const bool n2 = isnan(v2); \
  EXPECT_EQ(n1, n2); \
  if (!n1 && !n2) { EXPECT_NEAR(v1, v2, abs_error); } \
}

#include <model/cppfunction.h>
#include <model/cppfunction-odb.hxx>
#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

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
  {"MyClass::operator bool", 1},
  {"MyClass::operator unsigned int", 1},
  {"MyClass::method1", 8},
  {"NoBody1::NoBody1", 1},
  {"NoBody2::NoBody2", 1},
  {"NoBody3::~NoBody3", 1},
};

TEST_P(ParameterizedMcCabeTest, McCabeTest) {
  _transaction([&, this]() {
    typedef odb::query<model::CppFunction> QFun;
    model::CppFunction func = _db->query_value<model::CppFunction>(
      QFun::qualifiedName == GetParam().first && QFun::mccabe != 0);

    EXPECT_EQ(GetParam().second, func.mccabe);
  });
}

INSTANTIATE_TEST_SUITE_P(
  ParameterizedMcCabeTestSuite,
  ParameterizedMcCabeTest,
  ::testing::ValuesIn(paramMcCabe)
);

// Bumpy Road

struct BumpyRoadParam
{
  std::string funName;
  unsigned int expBumpiness;
  unsigned int expStmtCount;
};

class ParameterizedBumpyRoadTest
  : public CppMetricsParserTest,
    public ::testing::WithParamInterface<BumpyRoadParam>
{};

#define BR_LAM "::(anonymous class)::operator()"
#define BR_NM1 "::nested1::method1"
#define BR_NM2 "::nested2::method2"
#define BR_NM3 "::nested3::method3"
std::vector<BumpyRoadParam> paramBumpyRoad = {
  {"flat_empty_inline", 0, 0},
  {"flat_empty", 0, 0},
  {"flat_regular", 4, 4},
  {"single_compound", 3, 2},
  {"single_if_simple", 4, 3},
  {"single_if_complex", 4, 3},
  {"single_for_each", 5, 4},
  {"single_for_loop", 5, 4},
  {"nested_chain_compound", 10, 4},
  {"nested_chain_if", 10, 4},
  {"nested_chain_compound_if", 10, 4},
  {"nested_chain_for", 10, 4},
  {"nested_chain_compound_for", 10, 4},
  {"nested_chain_mixed", 29, 8},
  {"compare_level1", 7, 4},
  {"compare_level2", 12, 5},
  {"compare_level3", 18, 6},
  {"complex_two_levels", 17, 10},
  {"complex_three_levels_min", 14, 8},
  {"complex_three_levels_max", 23, 8},
  {"nested_lambda()" BR_LAM "()" BR_LAM "()" BR_LAM, 1, 1},
  {"nested_lambda()" BR_LAM "()" BR_LAM, 3, 2},
  {"nested_lambda()" BR_LAM, 6, 3},
  {"nested_lambda", 10, 4},
  {"nested_type()" BR_NM1 "()" BR_NM2 "()" BR_NM3, 1, 1},
  {"nested_type()" BR_NM1 "()" BR_NM2, 3, 2},
  {"nested_type()" BR_NM1, 6, 3},
  {"nested_type", 10, 4},
};

TEST_P(ParameterizedBumpyRoadTest, BumpyRoadTest) {
  _transaction([&, this]() {
    typedef odb::query<model::CppFunction> QFun;
    model::CppFunction func = _db->query_value<model::CppFunction>(
      QFun::qualifiedName == GetParam().funName);

    EXPECT_EQ(GetParam().expBumpiness, func.bumpiness);
    EXPECT_EQ(GetParam().expStmtCount, func.statementCount);
  });
}

INSTANTIATE_TEST_SUITE_P(
  ParameterizedBumpyRoadTestSuite,
  ParameterizedBumpyRoadTest,
  ::testing::ValuesIn(paramBumpyRoad)
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


// Relational cohesion
struct RelationalCohesionParam
{
  std::string moduleName;
  double rc;
};

class ParameterizedRelationalCohesionTest
  : public CppMetricsParserTest,
    public ::testing::WithParamInterface<RelationalCohesionParam>
{};

std::vector<RelationalCohesionParam> paramRelationalCohesion = {
  {"OneTypeInModule", 1.0}
  /*{"MultipleTypesInModuleNoRelation", 0.25},
    {"RelationByFunctionParameter", 7.0/4.0},
    {"RelationByFunctionReturnValue", 6.0/4.0},
    {"RelationByVariable", 6.0/4.0}*/
};

TEST_P(ParameterizedRelationalCohesionTest, RelationalCohesionTest) {
  _transaction([&, this](){
    const std::string& moduleName = GetParam().moduleName;
    std::uint64_t moduleId = _db->query_value<model::File>(
      !odb::query<model::File>::path.like("%/CMakeFiles/%") &&
      odb::query<model::File>::filename == moduleName).id;
    double storedValue = _db->query_value<model::CppFileMetrics>(
      odb::query<model::CppFileMetrics>::file == moduleId &&
      odb::query<model::CppFileMetrics>::type == model::CppFileMetrics::Type::RELATIONAL_COHESION
    ).value;

    constexpr double tolerance = 1e-8;
    EXPECT_EQF(GetParam().rc, storedValue, tolerance);
  });
}

INSTANTIATE_TEST_SUITE_P(
  ParameterizedRelationalCohesionTestSuite,
  ParameterizedRelationalCohesionTest,
  ::testing::ValuesIn(paramRelationalCohesion)
);