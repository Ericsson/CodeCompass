#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <set>
#include <string>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <model/cxx/cppastnode.h>
#include <model/asttype.h>

#include <cppservicehelper/symbolhandler.h>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

//TODO: test inherited by, inherits from, and friends (firend class, and friend function)

using namespace cc::service::language;
using namespace cc::service::core;
using namespace cc;

class SymbolHandlerTest : public ::testing::Test
{
protected:  
  std::shared_ptr<odb::database> _db;
  
  SymbolHandlerTest() {}
  
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

TEST_F(SymbolHandlerTest, Variablehandler)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  std::unique_ptr<SymbolHandler> symbolHandler = SymbolHandler::getHandler(_db, model::CppAstNode::SymbolType::Variable);
  
  model::CppAstNode variable;
  
  {
    odb::transaction t(_db->begin());

    typedef odb::query<model::CppAstNode>  AQuery;
    typedef odb::result<model::CppAstNode> AResult;

    auto res (_db->query<model::CppAstNode>(
      AQuery::astValue == "locaVar2"
    ));      
    
    variable = *res.begin();
  }
  
  {
    odb::transaction t(_db->begin());
    std::vector<InfoNode> infonodes = symbolHandler->getInfoTree(variable);
    
    int hitcounter = 0;
    for(const InfoNode& node: infonodes)
    {
      if(node.label == "Name")
      {
        EXPECT_EQ(std::string("locaVar2"), node.value);
        ++hitcounter;              
      }
      else if(node.label == "Qualified Name")
      {
        EXPECT_EQ(std::string("locaVar2"), node.value);
        ++hitcounter;
      }
      else if(node.label == "Type")
      {
        EXPECT_EQ(std::string("LocalClass"), node.astValue.astNodeValue);
        ++hitcounter;
      }
      else if(node.label == "Declaration")
      {
        EXPECT_EQ(std::string("simple.cpp:42:3"), node.value);
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Reads")
      {
        std::vector<InfoNode> reads = symbolHandler->getSubInfoTree(variable, node.query);        
        EXPECT_EQ(0, reads.size());
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Writes")
      {
        std::vector<InfoNode> writes_bound = symbolHandler->getSubInfoTree(variable, node.query);
        EXPECT_EQ("simple.cpp (2)", writes_bound.front().category.front());
        std::vector<InfoNode> writes = symbolHandler->getSubInfoTree(variable, writes_bound.front().query);
        std::set<std::string> expected{"  locaVar2.getPrivX();", "  locaVar2.setPrivX(0);"};
        std::set<std::string> expLabels{"43:3", "44:3"};
        
        for(const InfoNode& node: writes)
        {          
          EXPECT_TRUE(expected.count(node.value) > 0);
          EXPECT_TRUE(expLabels.count(node.label) > 0);
        }
        EXPECT_EQ(expected.size(), writes.size());
        ++hitcounter;
      }
    }    
    EXPECT_EQ(6, hitcounter);            
  }  
}

// FIXME: WTF this is same as in the cppservicetest
TEST_F(SymbolHandlerTest, Typehandler)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");  
  std::unique_ptr<SymbolHandler> typeHandler = SymbolHandler::getHandler(_db, model::CppAstNode::SymbolType::Type);
  
  model::CppAstNode type;
  
  {
    odb::transaction t(_db->begin());

    typedef odb::query<model::CppAstNode>  AQuery;
    typedef odb::result<model::CppAstNode> AResult;

    auto res (_db->query<model::CppAstNode>(
      AQuery::astValue == "LocalClass" &&
      AQuery::symbolType == model::CppAstNode::SymbolType::Type &&
      AQuery::astType == model::CppAstNode::AstType::Definition
    ));      
    
    type = *res.begin();
  }
  
  {
    odb::transaction t(_db->begin());
    std::vector<InfoNode> infonodes = typeHandler->getInfoTree(type);
    
    int hitcounter = 0;
    for(const InfoNode& node: infonodes)
    {
      if(node.label == "Qualified Name")
      {
        EXPECT_EQ(std::string("(anonymous)::LocalClass"), node.value);
        ++hitcounter;
      }
      else if(node.label == "Defined")
      {
        EXPECT_EQ(std::string("simple.cpp:4:1"), node.value);
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Inherits From")
      {
        std::vector<InfoNode> inherits = typeHandler->getSubInfoTree(type, node.query);        
        EXPECT_EQ(0, inherits.size());
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Inherited By")
      {
        std::vector<InfoNode> inherited = typeHandler->getSubInfoTree(type, node.query);        
        EXPECT_EQ(0, inherited.size());
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Friends")
      {
        std::vector<InfoNode> friends = typeHandler->getSubInfoTree(type, node.query);        
        EXPECT_EQ(0, friends.size());
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Methods")
      {
        std::vector<InfoNode> subsets = typeHandler->getSubInfoTree(type, node.query);
        ASSERT_EQ(6, subsets.size()); //methods and Inherited
        std::set<std::string> expected{"void", "int", "[ignored]"};
        std::set<std::string> expLabels
        {
          "LocalClass()",
          "~LocalClass()",
          "getPrivX()",
          "setPrivX(int)",
        };
        for(const InfoNode& node: subsets)
        {
          if(node.category.front() == "public")
          {
            EXPECT_TRUE(expected.count(node.value) > 0);
            EXPECT_TRUE(expLabels.count(node.label) > 0);
          }
          else if( node.category.front() != "Inherited")
          {
            EXPECT_TRUE(false);
          }
            
        }        
        ++hitcounter;
      }
      else if(!node.category.empty() && node.category.front() == "Members")
      {
        std::vector<InfoNode> subsets = typeHandler->getSubInfoTree(type, node.query);
        ASSERT_EQ(2, subsets.size()); //privX, Inherited        
        EXPECT_EQ("private", subsets[0].category.front());
                        
        EXPECT_EQ("_privX",subsets[0].label);
        EXPECT_EQ("int",subsets[0].value);                   
        
        EXPECT_EQ("Inherited", subsets[1].category.front());
        
        ++hitcounter;
      }
    }    
    EXPECT_EQ(7, hitcounter);            
  }  
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
