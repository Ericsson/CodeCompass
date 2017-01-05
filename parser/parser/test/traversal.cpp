#include <memory>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>

#include <model/workspace.h>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <cxxparser/cxxparser.h>

#include <util/util.h>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;

class TestTraversal : public Traversal
{
public:
  virtual DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_) override
  {
    return [&srcMgr_](const std::string& path_, FileType)
    {
      if(needContent(path_))
        return srcMgr_.addFile(path_);
      else
        return srcMgr_.addFile(path_, SourceManager::NoContent);
    };
  }

  static bool needContent(const std::string& path_)
  {
    const std::string fn = getFilename(path_);
    const std::string ext = getExtension(path_);
    return
        ext == "txt" ||
        ext == "md" ||
        ext == "xml" ||
        fn == "README" ||
        fn == "Makefile";
  }
};

TEST(TraversalTest, First)
{
  unlink("traversal_test.sqlite");
  std::shared_ptr<Workspace> w = Workspace::getCreateWorkspace(
    "sqlite:database=traversal_test.sqlite", Workspace::Create);
  {
    ParseProps props = ProjectParser::createParseProperties(w);

    SourceManager srcMgr(w, props);

    Parser& parser = Parser::getParser();
    std::shared_ptr<GeneralProjectParser> gpp = std::make_shared<GeneralProjectParser>(w, props, srcMgr);
    std::shared_ptr<TestTraversal> fstraversal(new TestTraversal());
    gpp->registerTraversal(fstraversal);

    parser.registerProjectParser(gpp);
    //CXXParser cxxParser(w);
    //FileParser::instance()->registerParser(&cxxParser);
    parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/traversal");

  }

  {
    cc::model::transaction t(w->getTransaction());

    typedef odb::query<File>  FQuery;
    typedef odb::result<File> FResult;

    FResult res ( w->getDb()->query<File>(FQuery::type == File::GenericFile));
    int cnt = 0;
    for(FResult::iterator i(res.begin()); i != res.end(); ++i)
      ++cnt;
    EXPECT_EQ(5, cnt);

    typedef odb::result<FileContent> FCResult;

    FCResult res2 ( w->getDb()->query<FileContent>());
    cnt = 0;
    for(FCResult::iterator i(res2.begin()); i != res2.end(); ++i)
      ++cnt;
    EXPECT_EQ(4, cnt);
  }
}

int main(int argc, char** argv) {
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
