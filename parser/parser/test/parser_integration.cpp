#include <gtest/gtest.h>

#include <model/workspace.h>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <projectparser/xmlprojectparser.h>
#include <cxxparser/cxxparser.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;

class ParserIntegrationTest : public ::testing::Test
{
protected:
  std::shared_ptr<SourceManager>        _srcMgr;
  std::shared_ptr<CXXParser>            _parserCXX;
  std::shared_ptr<GeneralProjectParser> _parserGeneral;
  std::shared_ptr<XmlProjectParser>     _parserXml;
  std::shared_ptr<Workspace>            _workspace;
  FileParser&                           _fileParser;
  Parser&                               _parser;

protected:
  ParserIntegrationTest() :
    _workspace(0),
    _fileParser(FileParser::instance()),
    _parser(Parser::getParser())
  {
  }

  virtual void TearDown()
  {
    _fileParser.deregisterParser(_parserCXX);
    _parser.deregisterProjectParser(_parserGeneral);
    _parser.deregisterProjectParser(_parserXml);
    _parserCXX.reset();
    _parserGeneral.reset();
    _parserXml.reset();
    _workspace.reset();
    
  }

  void init(const char* dbname)
  {
    unlink(dbname);

    _workspace = Workspace::getCreateWorkspace(std::string("sqlite:database=")
      + dbname, Workspace::Create);
    ParseProps pp = ProjectParser::createParseProperties(_workspace);

    _srcMgr = std::make_shared<SourceManager>(_workspace, pp);
    _parserCXX.reset(new CXXParser(_workspace));
    _parserGeneral.reset(new GeneralProjectParser(_workspace, pp, *_srcMgr));
    _parserXml.reset(new XmlProjectParser(_workspace, pp, *_srcMgr));

    _fileParser.registerParser(_parserCXX);

    _parser.registerProjectParser(_parserGeneral);
    _parser.registerProjectParser(_parserXml);
  }
};

TEST_F(ParserIntegrationTest, GeneralParser)
{
  init("parser_integration_test.sqlite");

  _parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/tinyxml/");

  EXPECT_EQ(1, 1);
}

TEST_F(ParserIntegrationTest, XmlParser)
{
  char oldwd[PATH_MAX];

  init("parser_integration_test.sqlite");

  ::getcwd(oldwd, sizeof(oldwd));

  try
  {
    ::chdir(TOP_SRCDIR "parser/parser/test/test_src/");

    EXPECT_EQ(_parser.parseProject("tinyxml.xml"), true);

    ::chdir(oldwd);
  }
  catch (...)
  {
    ::chdir(oldwd);

    throw;
  }
}

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
