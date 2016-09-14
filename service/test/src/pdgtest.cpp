#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <set>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <core-api/common_types.h>

#include "../../cppservice/src/cppservice.h"

#include "reparser/reparser.h"
#include "../../cppservicehelper/src/slicer/pdg.h"
#include "../../cppservicehelper/src/slicer/slicerhelper.h"

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::service::language;
using namespace cc::service;
using namespace cc;

class Finder : public clang::RecursiveASTVisitor<Finder>
{  
public:
  Finder(const std::string& targetStr_) : targetStr(targetStr_), target(nullptr) {}
  
  bool VisitNamedDecl(clang::NamedDecl *decl)
  {
    if(decl->getNameAsString() == targetStr)
    {
      target = decl;
      return false; //found it, don't need further traversal
    }
    
    return true;
  }
  
  clang::Decl* result() { return target; }
  
private:
  std::string targetStr;
  clang::Decl* target;
};

class PDGTest : public ::testing::Test
{
protected:  
  std::shared_ptr<odb::database> _db;  
  
  PDGTest() {}
  
  virtual void TearDown()
  {    
    _db.reset();
  }
  
  void init(const char* dbname)
  {
    std::string connStr = dbname;    
    _db = util::createDatabase(connStr);   
  }
  
  model::FilePtr getFile(const std::string& filename)
  {
    model::FilePtr file;  

    odb::transaction t(_db->begin());
    
    typedef odb::query<model::File>  FQuery;   

    auto res (_db->query<model::File>(
      FQuery::filename == filename
    )); 
  
    file.reset( new model::File(*res.begin()));
    
    std::cerr << "DEBUG from test: file id: " << file->id << std::endl;
    
    return file;
  }
  
  std::pair<clang::Decl*, std::unique_ptr<clang::ASTUnit>>
  getFunction(const std::string& filename, 
              const std::string& functionname, 
              std::map<std::string, std::string>& virtualFileStorage )
  {
    model::FilePtr file = getFile(filename);
    
    parser::Reparser reparser(this->_db, virtualFileStorage);
    auto ast = reparser.createAst(file);
    
    Finder visitor(functionname);
    visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());
    
    return std::make_pair(
      visitor.result(), std::unique_ptr<clang::ASTUnit>(std::move(ast)));
  }
  
  
  core::FilePosition createFilePosition(const std::string& filename, int line, int col)
  {
    model::FilePtr file = getFile(filename);
    
    core::FilePosition filePos;
    filePos.file.fid = std::to_string(file->id);
    filePos.pos.line = line;
    filePos.pos.column = col;
    
    return filePos;
  }
  
  void checkResult(const std::vector<core::Range>& result, 
                   const std::set<int>& expected, 
                   int expected_count = -1)
  {
    int cnt = 0;
    if(expected_count == -1) 
       expected_count = expected.size();;    
  
    for( const core::Range& r : result)
    {
      if(int res = expected.count(r.startpos.line))
      {
        ++cnt;
        EXPECT_EQ(res, 1);
      }
    }
        
    EXPECT_EQ(cnt,expected_count);
  }
};


TEST_F(PDGTest, first)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "first", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  pdg.dump2dot(std::cout);        
  
  core::FilePosition filePos = createFilePosition("first.cpp", 23, 6);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos); 
  
  checkResult( result, {5,6,7,9,12,13,15,17,18,20,23} );    
}

TEST_F(PDGTest, SlicerHelper)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
  SlicerHelper slicerHelper(_db);
  
  core::FilePosition filePos = createFilePosition("first.cpp", 23, 6);      
  
  std::vector<core::Range> result;  
  slicerHelper.getBackwardSlicePos(result, filePos);   
  
  checkResult( result, {5,6,7,9,12,13,15,17,18,20,23} );  
}


TEST_F(PDGTest, service)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
  
  CppServiceHelper cppserviceHelper(_db);
  CppServiceHandler cppservice(cppserviceHelper);  
  
  core::FilePosition filePos = createFilePosition("first.cpp", 23, 6);    
  
  std::vector<core::Range> result;  
  
  cppservice.getBackwardSlicePos(result, filePos);
  
  checkResult( result, {5,6,7,9,12,13,15,17,18,20,23} );
    
}

TEST_F(PDGTest, for_)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "for_test", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 51, 7);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);  
  
  for(auto x : result)
  {
    std::cerr << x.startpos.line << " ";
  }
  
  checkResult( result, {48,49,51}, 5 );  
}

TEST_F(PDGTest, increment_)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "incrementtest", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 40, 3);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);    
  checkResult( result, {32, 35, 40} );  
  result.clear();
  
  core::FilePosition filePos2 = createFilePosition("first.cpp", 41, 4);    
  pdg.backwardSlicePositions(result, filePos2);  
  checkResult( result, {33, 36, 41} );  
}

TEST_F(PDGTest, argument)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "argument", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 97, 5);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);    
  checkResult( result, {94, 96, 97} );    
}

TEST_F(PDGTest, decltest2)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "decltest2", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 112, 5);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);    
  checkResult( result, {109, 110, 111, 112} );    
}

TEST_F(PDGTest, returntest2)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "returntest2", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 138, 10);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);    
  checkResult( result, {130, 132, 133, 137, 138}, 6 );  
  result.clear();
  
  core::FilePosition filePos2 = createFilePosition("first.cpp", 137, 7);    
  pdg.backwardSlicePositions(result, filePos2);  
  checkResult( result, {130, 132, 133, 137} );  
}

TEST_F(PDGTest, forward_argument)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "argument", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 93, 7);    
  
  std::vector<core::Range> result;  
  pdg.forwardSlicePositions(result, filePos);    
  checkResult( result, {93} );  
  result.clear();
  
  core::FilePosition filePos2 = createFilePosition("first.cpp", 94, 3);    
  pdg.forwardSlicePositions(result, filePos2);  
  checkResult( result, {94,95,96,97} );  
}

TEST_F(PDGTest, forward_first)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pdg/pdg.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("first.cpp", "first", virtualFileStorage);
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("first.cpp", 18, 10);    
  
  std::vector<core::Range> result;  
  pdg.forwardSlicePositions(result, filePos);    
  checkResult( result, {9,11,12,13,15,17,18,20,23}, 10 );   
 
}

TEST_F(PDGTest, tinyxml_XMLTest_native)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");  
    
  PDG pdg(_db);
  std::map<std::string, std::string> virtualFileStorage;
  auto x = getFunction("xmltest.cpp", "XMLTest", virtualFileStorage);
  
  pdg.build(x.first, &x.second->getASTContext().getSourceManager());
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("xmltest.cpp", 43, 13);    
  
  std::vector<core::Range> result;  
  pdg.backwardSlicePositions(result, filePos);
  checkResult( result, {43} );  
  
}

//*/

TEST_F(PDGTest, tinyxml_XMLTest_interface)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");  
    
  //PDG pdg(_db);
  //auto x = getFunction("xmltest.cpp", "XMLTest");
  CppServiceHelper cppserviceHelper(_db);
  CppServiceHandler cppservice(cppserviceHelper);  
  
  //pdg.build(x.first, x.second);  
  //pdg.dump2dot(std::cout);    
  
  core::FilePosition filePos = createFilePosition("xmltest.cpp", 43, 13);    
  
  std::vector<core::Range> result;  
  cppservice.getBackwardSlicePos(result, filePos);    
  checkResult( result, {43} );  
  
}
//*/


int main(int argc, char** argv) {
  util::StreamLog::setStrategy(std::shared_ptr<util::LogStrategy>(
      new util::StandardErrorLogStrategy()))  ;
  util::StreamLog::setLogLevel(util::INFO);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
