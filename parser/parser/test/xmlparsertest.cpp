#include <gtest/gtest.h>
#include <projectparser/xmlparser.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

// TODO: write validation tests

using namespace cc::parser;
using namespace cc::util;

#define _TP(path) TOP_SRCDIR path

class XmlParserTest : public ::testing::Test
{
public:
  XMLParser               parser;
  XMLParser::BuildActions actions;
  std::map<std::string, std::string> projectOptions;
};

TEST_F(XmlParserTest, NonExistentXml)
{
  ASSERT_FALSE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/foo.xml"),
    actions,
    projectOptions));
  
  ASSERT_EQ(0, projectOptions.size());
}

TEST_F(XmlParserTest, EmptyXml)
{
  ASSERT_FALSE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/empty.xml"),
    actions,
    projectOptions));
  
  ASSERT_EQ(0, projectOptions.size());
}
/* FIXME test cases below
TEST_F(XmlParserTest, NoDefaultConfig1)
{
  ASSERT_FALSE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/noconfig1.xml"),
    actions,
    projectOptions));
  
  ASSERT_EQ(0, projectOptions.size());
}


TEST_F(XmlParserTest, NoDefaultConfig2)
{
  ASSERT_FALSE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/noconfig2.xml"),
    actions,
    projectOptions));

  parser.setConfig("yy");
  ASSERT_TRUE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/noconfig2.xml"),
    actions,
    projectOptions));
  ASSERT_EQ(actions.actions.size(), 0);
}
*/
TEST_F(XmlParserTest, NoAction)
{
  ASSERT_TRUE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/noaction.xml"),
    actions,
    projectOptions));
  ASSERT_EQ(actions.actions.size(), 0);
}

TEST_F(XmlParserTest, EmptyAction)
{
  ASSERT_TRUE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/emptyaction.xml"),
    actions,
    projectOptions));
  ASSERT_EQ(actions.actions.size(), 1);
  ASSERT_EQ(actions.actions[0].label.empty(), true);
  ASSERT_EQ(actions.actions[0].sources.size(), 0);
  ASSERT_EQ(actions.actions[0].targets.size(), 0);
  ASSERT_EQ(actions.actions[0].options.size(), 0);
}

/* FIXME test cases below
TEST_F(XmlParserTest, BadRoot)
{
  ASSERT_FALSE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/badroot.xml"),
    actions,
    projectOptions));
}

TEST_F(XmlParserTest, OneAction)
{
  ASSERT_TRUE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/oneaction.xml"),
    actions,
    projectOptions));

  ASSERT_EQ(actions.actions.size(), 1);
  ASSERT_EQ(actions.actions[0].label, "action1");
  ASSERT_EQ(actions.actions[0].sources.size(), 1);
  ASSERT_EQ(actions.actions[0].targets.size(), 1);
  ASSERT_EQ(actions.actions[0].options.size(), 3);
  ASSERT_EQ(actions.actions[0].sources[0], "dir1/y/x.cpp");
  ASSERT_EQ(actions.actions[0].targets[0], "dir2/z/x.o");
  ASSERT_EQ(actions.actions[0].options[0], "-Wall");
  ASSERT_EQ(actions.actions[0].options[1], "-std=c++11");
  ASSERT_EQ(actions.actions[0].options[2], "-option_with_space arg");
}

TEST_F(XmlParserTest, TwoAction)
{
  ASSERT_TRUE(parser.parse(
    _TP("parser/parser/test/test_src/buildxml/twoaction.xml"),
    actions,
    projectOptions));

  ASSERT_EQ(actions.actions.size(), 2);

  ASSERT_EQ(actions.actions[0].label, "action1");
  ASSERT_EQ(actions.actions[0].sources.size(), 1);
  ASSERT_EQ(actions.actions[0].targets.size(), 1);
  ASSERT_EQ(actions.actions[0].options.size(), 3);
  ASSERT_EQ(actions.actions[0].sources[0], "dir1/y/x.cpp");
  ASSERT_EQ(actions.actions[0].targets[0], "dir2/z/x.o");
  ASSERT_EQ(actions.actions[0].options[0], "-Wall");
  ASSERT_EQ(actions.actions[0].options[1], "-std=c++11");
  ASSERT_EQ(actions.actions[0].options[2], "-option_with_space arg");

  ASSERT_EQ(actions.actions[1].label, "action2");
  ASSERT_EQ(actions.actions[1].sources.size(), 1);
  ASSERT_EQ(actions.actions[1].targets.size(), 1);
  ASSERT_EQ(actions.actions[1].options.size(), 3);
  ASSERT_EQ(actions.actions[1].sources[0], "dir1/y/x.cpp");
  ASSERT_EQ(actions.actions[1].targets[0], "dir2/z/x.o");
  ASSERT_EQ(actions.actions[1].options[0], "  -Wall");
  ASSERT_EQ(actions.actions[1].options[1], "-std=c++11 ");
  ASSERT_EQ(actions.actions[1].options[2], " -option_with_space arg");
}
*/
int main(int argc, char *argv[])
{
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
    new StandardErrorLogStrategy()));

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
