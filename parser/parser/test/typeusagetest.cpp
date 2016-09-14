#include <gtest/gtest.h>

#include <string>

#include <parser/parsertest.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;

namespace cc
{

const std::string TestSrc = TOP_SRCDIR "parser/parser/test/test_src/";

TEST_F(ParserTest, TypeUsageTest)
{
  init("typeusage.sqlite", TestSrc + "typeusage");

  auto db = _workspace->getDb();

  odb::transaction t(db->begin());

  auto typeA = loadEntity<model::CppType>("A");

  auto mnHash = typeA.mangledNameHash;

  auto globalUsage = countUsage(mnHash, model::CppAstNode::AstType::GlobalTypeLoc);
  auto localUsage  = countUsage(mnHash, model::CppAstNode::AstType::LocalTypeLoc);
  auto returnUsage = countUsage(mnHash, model::CppAstNode::AstType::ReturnTypeLoc);
  auto paramUsage  = countUsage(mnHash, model::CppAstNode::AstType::ParameterTypeLoc);
  auto fieldUsage  = countUsage(mnHash, model::CppAstNode::AstType::FieldTypeLoc);

  EXPECT_EQ(1, globalUsage);
  EXPECT_EQ(1, localUsage);
  EXPECT_EQ(1, returnUsage);
  EXPECT_EQ(2, paramUsage);
  EXPECT_EQ(1, fieldUsage);

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
