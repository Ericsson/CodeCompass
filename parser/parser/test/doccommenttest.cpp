#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>

#include <model/workspace.h>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>
#include <model/cxx/cppvariable.h>
#include <model/cxx/cppvariable-odb.hxx>
#include <model/cxx/cppfunction.h>
#include <model/cxx/cppfunction-odb.hxx>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <cxxparser/cxxparser.h>
#include <model/comment/doccomment.h>
#include <model/comment/doccomment-odb.hxx>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

#include <iostream>
#include <sstream>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;


class CommentParsingTest : public ::testing::Test
{
protected:
  std::shared_ptr<SourceManager>        _srcMgr;
  std::shared_ptr<CXXParser>            _parserCXX;
  std::shared_ptr<GeneralProjectParser> _parserGeneral;
  std::shared_ptr<Workspace>            _workspace;
  FileParser&                           _fileParser;
  Parser&                               _parser;

public:
   CommentParsingTest( ) :
     _workspace(0),
     _fileParser(FileParser::instance()),
     _parser(Parser::getParser())
   {
   }

   virtual void TearDown( ) {
     _fileParser.deregisterParser(_parserCXX);
     _parser.deregisterProjectParser(_parserGeneral);
     _parserCXX.reset();
     _parserGeneral.reset();
     _workspace.reset();
   }

  void init(const char* dbname)
  {
    unlink(dbname);

    _workspace = Workspace::getCreateWorkspace(
      std::string("sqlite:database=") + dbname, Workspace::Create);
    ParseProps pp = ProjectParser::createParseProperties(_workspace);

    _srcMgr = std::make_shared<SourceManager>(_workspace, pp);
    _parserCXX.reset(new CXXParser(_workspace));
    _parserGeneral.reset(new GeneralProjectParser(_workspace, pp, *_srcMgr));

    _fileParser.registerParser(_parserCXX);

    _parser.registerProjectParser(_parserGeneral);
  }

  odb::result<DocComment> getCommentsForId(std::string name)
  {
    typedef odb::query<CppFunction>  CQuery;
    typedef odb::result<CppFunction> CResult;
    typedef odb::query<DocComment>  DQuery;

    CResult res ( this->_workspace->getDb()->query<CppFunction>( CQuery::qualifiedName == name) );
    CResult::iterator it(res.begin());

    return ( this->_workspace->getDb()->query<DocComment>( DQuery::mangledNameHash == it->mangledNameHash) );
  }

  uint64_t getIdFromHtmlContent(std::string& contentHTML)
  {
    std::string searchstr("<span class=\"docCommentLink\" data-linkHash=\"");
    size_t startpos = contentHTML.find(searchstr);
    size_t endpos = contentHTML.find("\">", startpos);
    contentHTML = contentHTML.substr(startpos+searchstr.size(), endpos-startpos-searchstr.size());

    std::istringstream iss(contentHTML);
    uint64_t id;
    iss >> id;

    return id;
  }

  bool checkLinkingValidity(std::string fname, std::string linkedname)
  {
    typedef odb::result<DocComment> DResult;

    DResult res(getCommentsForId(fname));
    if (!res.empty())
    {
      DResult::iterator i(res.begin());

      uint64_t id = getIdFromHtmlContent(i->contentHTML);

      typedef odb::query<CppAstNode>  AQuery;
      typedef odb::result<CppAstNode> AResult;
      AResult res3 ( this->_workspace->getDb()->query<CppAstNode>( AQuery::id == id) );
      AResult::iterator it3(res3.begin());

      return it3->astValue == linkedname;
    }
    return false;
  }

};

// check different comment types
TEST_F (CommentParsingTest, ExistingTest1)
{
  init("doccommenttest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/doccomment/");
  cc::model::transaction t(_workspace->getTransaction());

  EXPECT_EQ(1, !getCommentsForId("Simple::f").empty());
}

TEST_F (CommentParsingTest, ExistingTest2)
{
  init("doccommenttest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/doccomment/");
  cc::model::transaction t(_workspace->getTransaction());

  EXPECT_EQ(1, !getCommentsForId("Simple::SimpleClass::g").empty());
}

//check function/class linking
TEST_F (CommentParsingTest, RefTest1)
{
  init("doccommenttest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/doccomment/");
  cc::model::transaction t(_workspace->getTransaction());

  EXPECT_EQ(1, checkLinkingValidity("Simple::f", "g"));
}

TEST_F (CommentParsingTest, RefTest2)
{
  init("doccommenttest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/doccomment/");
  cc::model::transaction t(_workspace->getTransaction());

  EXPECT_EQ(1, checkLinkingValidity("Simple::SimpleClass::SimpleClass", "g"));
}

// parameter test
TEST_F (CommentParsingTest, ParamTest)
{
  init("doccommenttest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/doccomment/");
  cc::model::transaction t(_workspace->getTransaction());

  EXPECT_EQ(1, checkLinkingValidity("Simple::SimpleClass::g", "a"));
}

int main(int argc, char** argv) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
