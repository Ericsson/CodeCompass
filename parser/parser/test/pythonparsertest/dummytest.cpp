#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include <util/environment.h>

#include "pythonparsertestfixture.h"
#include "python_testhelper.h"
#include "python_filetester.h"

namespace cc
{

const std::string TestSrc = TOP_SRCDIR "parser/parser/test/test_src/";

namespace test
{
namespace python
{

void testDb(odb::database& db_);

TEST_F(PythonFixture, DataTest)
{
  using namespace model;

  init("pythonparsertest.sqlite", TestSrc + "pythonparser/test_files/simple_test2/");

  auto db = _workspace->getDb();

  odb::transaction t(db->begin());

  auto result = db->query<PythonAstNode>();
  EXPECT_FALSE(result.empty());

  testDb(*db);

  t.commit();
}

void testDb(odb::database& db_)
{
  PythonTestHelper helper(db_);
  PythonFileTester ft(helper);

  // simple_test2

  const std::string filePath = TestSrc + "pythonparser/test_files/simple_test2/";

  EXPECT_TRUE(ft.checkAstNode_st2_highlight_variable(filePath));
  EXPECT_TRUE(ft.checkAstNode_st2_highlight_function(filePath));
  EXPECT_TRUE(ft.checkAstNodeContainerBinding_st2_attribute(filePath));
  EXPECT_TRUE(ft.checkClassAttrNum_st2_attribute(filePath));
  EXPECT_TRUE(ft.checkClassConstructor_st2_attribute(filePath));
  EXPECT_TRUE(ft.checkClassBases_st2_attribute(filePath));
  EXPECT_TRUE(ft.checkFunctionDefParams_st2_function(filePath));
  EXPECT_TRUE(ft.checkFunctionDefDecorators_st2_function(filePath));
  EXPECT_TRUE(ft.checkFunctionDefVariables_st2_function(filePath));
  EXPECT_TRUE(ft.checkUnknownNameAndTarget_st2_unknown_attr(filePath));
  EXPECT_TRUE(ft.checkUnknownNameAndTarget_st2_unknown_func(filePath));
}

} // python
} // test
} // cc

int main(int argc, char **argv)
{
  using namespace cc::util;

  cc::util::Environment::init();

  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
                           new StandardErrorLogStrategy()))  ;
  StreamLog::setLogLevel(INFO);

  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
