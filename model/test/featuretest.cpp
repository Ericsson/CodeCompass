#include <memory>

#include <gtest/gtest.h>
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/session.hxx>

#include <odb/connection.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>


#include <model/cxx/cppastnode.h>
#include <model/cxx/cppastnode-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>
#include <model/project.h>
#include <model/project-odb.hxx>

using namespace std;
using namespace cc::model;
using namespace odb::core;

static const char* str[] = {" ", "--user", "featuretest", "--database", "featuretest.sqlite"};
int argc = 5;

class FeatureSaveLoad : public ::testing::Test
{
public:

protected:
  FeatureSaveLoad() : db(new odb::sqlite::database(argc, const_cast<char**>(str), false,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE))
  {
    try
    {
      {
        connection_ptr c(this->db->connection());

        c->execute ("PRAGMA foreign_keys=OFF");

        transaction t (c->begin());
        schema_catalog::create_schema (*db);
        t.commit ();

        c->execute ("PRAGMA foreign_keys=ON");
      }

    }
    catch (const odb::exception& e)
    {
      std::cerr << e.what () << endl;
    }
  }

  ~FeatureSaveLoad()
  {
    //  unlink("featuretest.sqlite");
  }

  auto_ptr<database> db;

//   static void TearDownTestCase()
//   {
//     std::cout << "TearDownTestCase" << std::endl;
//   }


};


TEST_F(FeatureSaveLoad, CppAstNodePtr)
{
  try
  {
    {
      transaction t(this->db->begin());


      CppAstNode astNode;
      astNode.astValue = "X";


      // Make objects persistent and save their ids for later use.
      //
      this->db->persist(astNode);


      //Update
      odb::result<CppAstNode> r(db->query<CppAstNode>(odb::query<CppAstNode>::astValue == "X"));
      shared_ptr<CppAstNode> tmp(r.begin().load ());
      tmp->astValue = "XX";
      db->update (*tmp);

      t.commit();
    }

    {
      session s;
      transaction t(this->db->begin());

      odb::result<CppAstNode> r(db->query<CppAstNode>(odb::query<CppAstNode>::astValue == "XX"));
      CppAstNode astNode = *r.begin();
      EXPECT_EQ("XX", astNode.astValue);
      //EXPECT_EQ(AstNodePtr(), astNode.parent);

    }
  }
  catch (const odb::exception& e)
  {
    std::cerr << e.what () << endl;
    return;
  }
}

TEST_F(FeatureSaveLoad, Symbol)
{
  try
  {
    {
      transaction t(this->db->begin());

      std::shared_ptr<Project> p(new Project);
      this->db->persist(*p);

      std::shared_ptr<FileContent> content(new FileContent);
      content->content = "dummy content";
      content->hash = "dummy hash";
      this->db->persist(*content);

      std::shared_ptr<File> file(new File);
      file->type = File::Unknown;
      file->path = "a.o";
      file->timestamp = 0;
      file->project = p;
      file->content = content;
      this->db->persist(*file);

      t.commit();
    }

    {
      transaction t(this->db->begin());

      std::shared_ptr<Project> p(new Project);
      this->db->persist(*p);

      std::shared_ptr<FileContent> content(new FileContent);
      content->content = "dummy content2";
      content->hash = "dummy hash2";
      this->db->persist(*content);

      std::shared_ptr<File> file(new File);
      file->type = File::Unknown;
      file->path = "b.o";
      file->timestamp = 0;
      file->project = p;
      file->content = content;
      this->db->persist(*file);

      CppAstNode node;
      node.mangledName = "_Zfv";
      node.symbolType = CppAstNode::SymbolType::Function;

      this->db->persist(node);

      t.commit();
    }

    {
      //TODO rendes ellenorzesek
      session s;
      transaction t(this->db->begin());

      auto r(this->db->query<CppAstNode>(odb::query<CppAstNode>::mangledName=="_Zfv" &&
                                      odb::query<CppAstNode>::symbolType==CppAstNode::SymbolType::Function));

      EXPECT_FALSE(r.empty());
      //EXPECT_EQ("_Zfv", symbol.mangledName);

      /*odb::result<Symbol> r2(this->db->query<Symbol>(odb::query<Symbol>::mangledName=="_Zfv" && odb::query<Symbol>::context==2));
      Symbol symbol2 = *r2.begin();
      EXPECT_EQ(Symbol::GlobalVariable, symbol2.type);*/
    }
  }
  catch (const odb::exception& e)
  {
    std::cerr << e.what () << endl;
    return;
  }
}



int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::AddGlobalTestEnvironment(new SQLEnvironment);

  return RUN_ALL_TESTS();
}
