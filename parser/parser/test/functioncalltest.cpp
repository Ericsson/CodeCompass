#include <gtest/gtest.h>

#include <string>

#include <parser/parsertest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <model/cxx/cppfunction.h>
#include <model/cxx/cppfunction-odb.hxx>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;

namespace cc
{

const std::string TestSrc = TOP_SRCDIR "parser/parser/test/test_src/";

TEST_F(ParserTest, FunctionCallTest)
{
  using namespace model;

  init("functioncalls.sqlite", TestSrc + "functioncalls");

  auto db = _workspace->getDb();

  odb::transaction t(db->begin());

  auto funcF = loadEntity<CppFunction>("f");
  auto funcG = loadEntity<CppFunction>("g");
  auto funcFoo = loadEntity<CppFunction>("B::foo");

  auto fUsage   = countUsage(funcF.mangledNameHash, CppAstNode::AstType::Usage);
  auto gUsage   = countUsage(funcG.mangledNameHash, CppAstNode::AstType::Usage);
  auto fooUsage = countUsage(funcFoo.mangledNameHash, CppAstNode::AstType::Usage);

  auto fooVirtUsage = countUsage(funcFoo.mangledNameHash, CppAstNode::AstType::VirtualCall);

  EXPECT_EQ(1, fUsage);
  EXPECT_EQ(1, gUsage);
  EXPECT_EQ(0, fooUsage);
  EXPECT_EQ(1, fooVirtUsage);

  t.commit();
}

} // cc

int main(int argc, char** argv) {
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
      new StandardErrorLogStrategy()))  ;
  StreamLog::setLogLevel(INFO);

  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
