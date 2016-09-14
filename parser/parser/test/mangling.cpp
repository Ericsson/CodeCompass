#include <set>

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

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;


TEST(MangledNameTest, First)
{
  unlink("mangled_name_test.sqlite");

  std::shared_ptr<Workspace> w = Workspace::getCreateWorkspace(
    "sqlite:database=mangled_name_test.sqlite", Workspace::Create);

  {
    ParseProps props = ProjectParser::createParseProperties(w);

    SourceManager srcMgr(w, props);

    Parser& parser = Parser::getParser();
    parser.registerProjectParser(std::make_shared<GeneralProjectParser>(w, props, srcMgr));
    std::shared_ptr<CXXParser> cxxParser( std::make_shared<CXXParser>(w));
    FileParser::instance().registerParser(cxxParser);
    parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/mangledname/");

  }

  {
    //session s;
    cc::model::transaction t(w->getTransaction());

    typedef odb::query<CppAstNode>  AQuery;
    typedef odb::result<CppAstNode> AResult;

    AResult res2 ( w->getDb()->query<CppAstNode>(
      AQuery::location.file.is_not_null() &&
      AQuery::mangledName.is_not_null() &&
      AQuery::mangledName != ""));

    std::set<std::string> mangledNames;
    for(AResult::iterator i(res2.begin()); i != res2.end(); ++i)
      mangledNames.insert(i->mangledName);
    
    EXPECT_EQ(8, mangledNames.size());
  }
}

int main(int argc, char** argv) {
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
