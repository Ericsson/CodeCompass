// vim: set ts=2 tabstop=2 shiftwidth=2 softtabstop=2 tw=80 :

#include <memory>

#include <gtest/gtest.h>
#include <model/workspace.h>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>
#include <parser/commondefs.h>
#include <parser/sourcemanager.h>
#include <projectparser/generalprojectparser.h>

#include <util/streamlog.h>
#include <util/filesystem.h>
#include <util/standarderrorlogstrategy.h>

// Make eclipse happy
#ifndef TOP_SRCDIR
#define TOP_SRCDIR ""
#endif

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;


class SourceManagerTest : public ::testing::Test
{
public:
  std::shared_ptr<Workspace>            _workspace;
  std::shared_ptr<SourceManager>        _srcMan;
  ParseProps                            _props;


protected:
  SourceManagerTest()
  {
    unlink("srcmanagertest.sqlite");

    _workspace = Workspace::getCreateWorkspace(
      "sqlite:database=srcmanagertest.sqlite", Workspace::Create);
    _props = ProjectParser::createParseProperties(_workspace);
  }

  ~SourceManagerTest()
  {
  }
};

TEST_F(SourceManagerTest, SaveLoad)
{
  SourceManager sm(_workspace, _props);

  cc::model::transaction t(_workspace->getTransaction());
  t->tracer(odb::stderr_tracer);

  ASSERT_TRUE(sm.addFile(TOP_SRCDIR "parser/parser/test/test_src/first/a.cpp"));
  ASSERT_TRUE(sm.addFile(TOP_SRCDIR "parser/parser/test/test_src/first/a.cpp"));
  ASSERT_FALSE(sm.addFile(TOP_SRCDIR "parser/parser/test/test_src/first/c.cpp"));

  {
    // SourceManager now caches the FilePtrs. Because of that it returns
    // transient File objects and calling load() on a transient object
    // results in a segmentation fault!

    FilePtr file;    
    ASSERT_TRUE(sm.findFile(TOP_SRCDIR "parser/parser/test/test_src/first/a.cpp", file));
    ASSERT_NE(file.get(), nullptr);
    ASSERT_STREQ(unifyPath(file->path).c_str(), 
                 unifyPath(TOP_SRCDIR "parser/parser/test/test_src/first/a.cpp").c_str() );
    ASSERT_EQ(file->project->id, _props.project->id);
    ASSERT_NE(file->content.get(), nullptr);
    ASSERT_NE(file->timestamp, static_cast<std::uint64_t>(0));
    //ASSERT_EQ(file->content->content, "int main()\n{\n  return 42;\n}\n\n");
    ASSERT_EQ(file->content->content, "");
    
    ASSERT_FALSE(sm.findFile(TOP_SRCDIR "parser/parser/test/test_src/first/b.cpp", file));
    ASSERT_EQ(file.get(), nullptr);
    
    ASSERT_FALSE(sm.findFile(TOP_SRCDIR "parser/parser/test/test_src/first/c.cpp", file));
    ASSERT_EQ(file.get(), nullptr);

    ASSERT_TRUE(sm.addFile(TOP_SRCDIR "parser/parser/test/test_src/first/c.cpp",
                             SourceManager::NoContent));
    ASSERT_TRUE(sm.findFile(TOP_SRCDIR "parser/parser/test/test_src/first/c.cpp", file));
    ASSERT_NE(file.get(), nullptr);
    ASSERT_EQ(file->content.get(), nullptr);
  }

  t->commit();
};



int main(int argc, char *argv[])
{
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
    new StandardErrorLogStrategy()));

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
