#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <set>
#include <string>
#include <memory>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <model/file.h>

#include <cppservicehelper/cppservicehelper.h>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::service::language;
using namespace cc::service::core;
using namespace cc;

class CppServiceHelperTest : public ::testing::Test
{
protected:
  std::shared_ptr<CppServiceHelper> _cppserviceHelper;
  std::shared_ptr<odb::database> _db;
  
  CppServiceHelperTest() : _cppserviceHelper(nullptr) {}
  
  virtual void TearDown()
  {
    _cppserviceHelper.reset();
    _db.reset();
  }
  
  void init(const char* dbname)
  {
    std::string connStr = dbname;    
    _db = util::createDatabase(connStr);
    _cppserviceHelper.reset( new CppServiceHelper(_db) );
  }
  
  AstNodeInfo getAstNodeInfo(int line, int col, model::FileId fid_)
  {
    FileId fid;    
    fid.__set_fid(std::to_string(fid_));
    
    Position pos;
    pos.__set_line(line);
    pos.__set_column(col);
    
    FilePosition fp;
    fp.__set_file(fid);
    fp.__set_pos(pos);
    
    return  _cppserviceHelper->getAstNodeInfoByPosition(fp, std::vector<std::string>());
  }
  
  model::FileId getFileId(const std::string& fn)
  {
    odb::transaction t(_db->begin());

    typedef odb::query<model::File>  FQuery;

    auto res (_db->query<model::File>(
      FQuery::filename == fn
    ));
    
    return (*res.begin()).id;
  }
  
  void checkDefinition(int line, int col, model::FileId fid, int expected_line)
  {
    AstNodeInfo anFrom = getAstNodeInfo(line, col, fid); 
    AstNodeInfo anTo = _cppserviceHelper->getDefinition(anFrom.astNodeId)[0];    
    EXPECT_EQ(expected_line, anTo.range.range.startpos.line);
  }
  
  void checkCallers(int line, int col, model::FileId fid, std::vector<int> expected_lines, int exptected_value = 1)
  {
    FileId fileId;
    fileId.__set_fid(std::to_string(fid));
  
    AstNodeInfo anFrom = getAstNodeInfo(line, col, fid);
    std::vector<AstNodeInfo> results = 
        _cppserviceHelper->getCallerFunctions(anFrom.astNodeId, fileId);
    
    std::sort(results.begin(), results.end(), [](const auto& lhs, const auto& rhs){
      return lhs.range.range.startpos.line < rhs.range.range.startpos.line;
    });
    
    EXPECT_EQ(exptected_value, results.size());
    for(size_t i = 0; i < results.size(); ++i)
      EXPECT_EQ(expected_lines[i], results[i].range.range.startpos.line);
  }
};

TEST_F(CppServiceHelperTest, getDefinitionSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fileId = getFileId("simple.cpp");
  
  checkDefinition(24,13,fileId,33); // _privX
  checkDefinition(43,13,fileId,27); // localVar2.setPrivX(0); call
  checkDefinition(42,4,fileId,4); // LocalClass localVar2; type 
}

TEST_F(CppServiceHelperTest, getDefinitionTinyXml)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");  
  auto tinyxml2 = getFileId("tinyxml2.cpp");
  auto xmltest = getFileId("xmltest.cpp");
  
  checkDefinition(98,7,tinyxml2,178); // _end
  checkDefinition(125,27,tinyxml2,119); // length
  checkDefinition(117,41,xmltest,636); // FirstChild
  checkDefinition(257,41,xmltest,677); // InsertFirstChild
  checkDefinition(346,9,xmltest,1357); // LocalClass localVar2; type 
}

TEST_F(CppServiceHelperTest, getAstNodeInfoByPosition)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fileId = getFileId("simple.cpp");
    
  {
    AstNodeInfo ani = getAstNodeInfo(24, 13, fileId);  
    EXPECT_EQ("Variable",ani.astNodeType);
    EXPECT_EQ("(anonymous namespace)::LocalClass::_privX", ani.astNodeValue);
  }
  
  {
    AstNodeInfo ani = getAstNodeInfo(4, 7, fileId);
    EXPECT_EQ("Type",ani.astNodeType);
    EXPECT_EQ("LocalClass", ani.astNodeValue);
  }
  
  {
    AstNodeInfo ani = getAstNodeInfo(38, 6, fileId);
    EXPECT_EQ("Function",ani.astNodeType);
    EXPECT_EQ("main", ani.astNodeValue);
  }
  
  {
    AstNodeInfo ani = getAstNodeInfo(38, 15, fileId);
    EXPECT_EQ("Variable",ani.astNodeType);
    EXPECT_EQ("argc_", ani.astNodeValue);
  }
  
  {
    AstNodeInfo ani = getAstNodeInfo(38, 15, fileId);
    EXPECT_EQ("Variable",ani.astNodeType);
    EXPECT_EQ("argc_", ani.astNodeValue);
  }
  
  /* TODO: int x = 0; eseten az int-et kijelolve az x-et kapjuk vissza ez meg nincs jol implementalva
  {
    AstNodeInfo ani = getAstNodeInfo(40, 4);
    EXPECT_EQ("Type",ani.astNodeType);
    EXPECT_EQ("int", ani.astNodeValue);
  }
  */
  
  /* TODO: itt a return 0-an a return-re kattintottunk, de mivel csak azokat az astNode-okat taroljuk, amelyiknek
   * van mangled neve, a legszukebb igy a main lett
  {
    AstNodeInfo ani = getAstNodeInfo(46, 4);
    EXPECT_EQ("",ani.astNodeType);
    EXPECT_EQ("", ani.astNodeValue);
  }
  */

}

TEST_F(CppServiceHelperTest, getReferencesSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fid = getFileId("simple.cpp");
  FileId fileId;
  fileId.__set_fid(std::to_string(fid));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(33, 11, fid); // def of _privX        
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getReferences(anFrom.astNodeId, fileId);
    std::set<int> expected = {8,19,24,29,33}; //33 is a definition
    EXPECT_EQ(expected.size(), refs.size());
    
    for(const auto& ref : refs)
    {
      EXPECT_TRUE(expected.count(ref.range.range.startpos.line) != 0);
    }       
  }
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(27, 11, fid); // def of setPrivX   
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getReferences(anFrom.astNodeId, fileId);
    std::set<int> expected = {27,43}; 
    EXPECT_EQ(expected.size(), refs.size());
    
    for(const auto& ref : refs)
    {
      EXPECT_TRUE(expected.count(ref.range.range.startpos.line) != 0);
    }       
  }
}
/* TODO FIXME*/
TEST_F(CppServiceHelperTest, getReferencesTinyXml)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");  
  auto tinyxml2_h_id = getFileId("tinyxml2.h");
  FileId tinyxml2_h;
  tinyxml2_h.__set_fid(std::to_string(tinyxml2_h_id));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(143, 12, tinyxml2_h_id); // def of Set 
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getReferences(anFrom.astNodeId, tinyxml2_h);    
    EXPECT_EQ(1u, refs.size());
    
    EXPECT_EQ(143, refs[0].range.range.startpos.line);

  }
}

TEST_F(CppServiceHelperTest, getCallerFunctionsSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fid = getFileId("simple.cpp");
  FileId fileId;
  fileId.__set_fid(std::to_string(fid));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(27, 11, fid); // def of setPrivX   
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getCallerFunctions(anFrom.astNodeId, fileId);
    EXPECT_EQ(1u, refs.size());
    
    EXPECT_EQ(43, refs[0].range.range.startpos.line);
  }
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(17, 11, fid); // def of getPrivX   
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getCallerFunctions(anFrom.astNodeId, fileId);
    EXPECT_EQ(1u, refs.size());
    
    EXPECT_EQ(44, refs[0].range.range.startpos.line);
  }
}

TEST_F(CppServiceHelperTest, getFunctionCallsTiny)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");
  auto fid = getFileId("tinyxml2.h");
  FileId fileId;
  fileId.__set_fid(std::to_string(fid));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(143, 12, fid); // def of Set 
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getFunctionCalls(anFrom.astNodeId);
    std::set<int> expected = {144}; // Reset call
    EXPECT_EQ(expected.size(), refs.size());
    
    for(const auto& ref : refs)
    {
      EXPECT_TRUE(expected.count(ref.range.range.startpos.line) != 0);
    }       
  }
}

TEST_F(CppServiceHelperTest, getFunctionCalls)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fid = getFileId("simple.cpp");
  FileId fileId;
  fileId.__set_fid(std::to_string(fid));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(38, 7, fid); // def of main
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getFunctionCalls(anFrom.astNodeId);
    std::set<int> expected = {42, 43, 44}; 
    EXPECT_EQ(expected.size(), refs.size());
    
    for(const auto& ref : refs)
    {
      EXPECT_TRUE(expected.count(ref.range.range.startpos.line) != 0);
    }       
  }
}

TEST_F(CppServiceHelperTest, getInfoTree)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  auto fid = getFileId("simple.cpp");
  FileId fileId;
  fileId.__set_fid(std::to_string(fid));
  
  {
    AstNodeInfo anFrom = getAstNodeInfo(4, 7, fid); // def of LocalClass
    std::vector<InfoNode> its = _cppserviceHelper->getInfoTree(anFrom.astNodeId);
    
    /*for( InfoNode& in : its)
    {    
      for(std::string& str : in.category)
      {
        std::cout << str << " ";
      }
      std::cout << std::endl << in.label << std::endl << in.value << std::endl;
      std::cout << in.astValue.astNodeType << "\n" << in.astValue.astNodeValue << std::endl;
      std::cout<< "----------" << std::endl;
    }
    */
    
    EXPECT_EQ("Name", its[0].label);
    EXPECT_EQ("LocalClass", its[0].value);
    
    EXPECT_EQ("Qualified Name", its[1].label);
    EXPECT_EQ("(anonymous)::LocalClass", its[1].value);
  }

  
}

TEST_F(CppServiceHelperTest, getCallerFunctions)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/cpptest/cpptest.sqlite");
  auto fid = getFileId("cpptest.cpp");

  checkCallers(173, 6, fid, {170, 173, 176}, 3);
  checkCallers(178, 6, fid, {170, 176, 178}, 3);
}

int main(int argc, char** argv) {
  util::StreamLog::setStrategy(std::shared_ptr<util::LogStrategy>(
      new util::StandardErrorLogStrategy()))  ;
  util::StreamLog::setLogLevel(util::INFO);
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
