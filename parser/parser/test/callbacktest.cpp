#include <gtest/gtest.h>

#include <iostream>

#include <model/workspace.h>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <cxxparser/cxxparser.h>
#include <cxxparser/nullparser.h>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;


static int max = 0;
static int parsed = 0;
void progress(int a, int b)
{
  max = b;
  parsed = a;
}

TEST(Callback, First)
{
  {
    unlink("callback_test.sqlite");
    std::shared_ptr<Workspace> w = Workspace::getCreateWorkspace(
      "sqlite:database=callback_test.sqlite", Workspace::Create);

    ParseProps props = ProjectParser::createParseProperties(w);

    SourceManager srcMgr(w, props);

    Parser& parser = Parser::getParser();
    parser.registerProjectParser(std::make_shared<GeneralProjectParser>(w, props, srcMgr));
    parser.setProgressCallback(progress);
    std::shared_ptr<CXXNullParser> nullparser = std::make_shared<CXXNullParser>();
//     MockCXXParser cxxParser(w);
//     EXPECT_CALL(cxxParser, parse(_,_,_)).Times(1);
    FileParser::instance().registerParser(nullparser);
    parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/tinyxml/");

  }
  EXPECT_EQ(2, max);
  EXPECT_EQ(2, parsed);

}


int main(int argc, char** argv) {
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
