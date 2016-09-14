#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>

#include <model/workspace.h>
#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <cxxparser/cxxparser.h>

#include <unordered_set>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;

TEST(Templatetest, First)
{
  unlink("templatetest.sqlite");
  std::shared_ptr<Workspace> w = Workspace::getCreateWorkspace(
    "sqlite:database=templatetest.sqlite", Workspace::Create);

  {
    ParseProps props = ProjectParser::createParseProperties(w);

    SourceManager srcMgr(w, props);

    Parser& parser = Parser::getParser();
    parser.registerProjectParser(std::shared_ptr<GeneralProjectParser>(new GeneralProjectParser(w, props, srcMgr)));
    std::shared_ptr<CXXParser> cxxParser( new CXXParser(w) );
    FileParser::instance().registerParser(cxxParser);
    parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/template/");

  }

  {
    //session s;
    cc::model::transaction t(w->getTransaction());

    typedef odb::query<CppAstNode>  AQuery;
    typedef odb::result<CppAstNode> AResult;

    std::unordered_set<std::string> fooMangled;
    AResult ares( w->getDb()->query<CppAstNode>(AQuery::astValue == "foo" && ! (AQuery::mangledName == "") ));
    for (const auto& node : ares)
    {
      fooMangled.emplace(node.mangledName);
    }

    int cnt = 0;
    AResult ares2( w->getDb()->query<CppAstNode>(AQuery::astValue == "template call foo") );
    for(AResult::iterator i(ares2.begin()); i != ares2.end(); ++i)
    {
      ++cnt;
      std::cerr << "id: " << i->mangledName << std::endl;
      EXPECT_EQ(1, fooMangled.count(i->mangledName));
    }
    EXPECT_EQ(2, cnt);
  }
}

int main(int argc, char** argv) {
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
