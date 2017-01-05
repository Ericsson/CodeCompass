#include <set>
#include <string>

#include <gtest/gtest.h>
// #include <gmock/gmock.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include "reparser/reparser.h"

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc;

class TestVisitor : public clang::RecursiveASTVisitor<TestVisitor>
{
public:
  bool VisitNamedDecl(clang::NamedDecl *decl)
  {
    std::cout << decl->getNameAsString() << std::endl;
    return true;
  }
};

class ReparserTest : public ::testing::Test
{
protected:  
  std::shared_ptr<odb::database> _db;  
  
  ReparserTest() {}
  
  virtual void TearDown()
  {    
    _db.reset();
  }
  
  void init(const char* dbname)
  {
    std::string connStr = dbname;    
    _db = util::createDatabase(connStr);   
  }
  
};

TEST_F(ReparserTest, Dummy)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple_xmlproj/simplexml.sqlite");  
  
  model::FilePtr file;
  
  {
    odb::transaction t(_db->begin());
    
    typedef odb::query<model::File>  FQuery;   

    auto res (_db->query<model::File>(
      FQuery::filename == std::string("test.cpp")
    )); 
  
    file.reset( new model::File(*res.begin()));
  }
  
  std::map<std::string, std::string> virtualFileStorage;
  parser::Reparser reparser(this->_db, virtualFileStorage);
  auto ast = reparser.createAst(file);
  TestVisitor visitor;
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
  EXPECT_EQ(1,1);
}

int main(int argc, char** argv) {
  util::StreamLog::setStrategy(std::shared_ptr<util::LogStrategy>(
      new util::StandardErrorLogStrategy()))  ;
  util::StreamLog::setLogLevel(util::INFO);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
