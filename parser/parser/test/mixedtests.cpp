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
#include <model/cxx/cpptype.h>
#include <model/cxx/cpptype-odb.hxx>
#include <parser/parser.h>
#include <projectparser/generalprojectparser.h>
#include <cxxparser/cxxparser.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::parser;
using namespace cc::util;
using namespace cc::model;


class AstNodePropertyTests : public ::testing::Test
{
protected:
  std::shared_ptr<SourceManager>        _srcMgr;
  std::shared_ptr<CXXParser>            _parserCXX;
  std::shared_ptr<GeneralProjectParser> _parserGeneral;
  std::shared_ptr<Workspace>            _workspace;
  FileParser&                           _fileParser;
  Parser&                               _parser;

protected:
  AstNodePropertyTests() :
    _workspace(0),
    _fileParser(FileParser::instance()),
    _parser(Parser::getParser())
  {
  }

  virtual void TearDown()
  {
    _fileParser.deregisterParser(_parserCXX);
    _parser.deregisterProjectParser(_parserGeneral);    
    _parserCXX.reset();
    _parserGeneral.reset();
    _workspace.reset();
  }

  void init(const char* dbname)
  {
    unlink(dbname);

    _workspace = Workspace::getCreateWorkspace(std::string("sqlite:database=")
      + dbname, Workspace::Create);
    ParseProps pp = ProjectParser::createParseProperties(_workspace);

    _srcMgr = std::make_shared<SourceManager>(_workspace, pp);
    _parserCXX.reset(new CXXParser(_workspace));
    _parserGeneral.reset(new GeneralProjectParser(_workspace, pp, *_srcMgr));

    _fileParser.registerParser(_parserCXX);

    _parser.registerProjectParser(_parserGeneral);
  }
};


TEST_F(AstNodePropertyTests, AllVariableDefinitions)
{
  init("local_global_flag_test.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/mangledname/");

  //session s;
  cc::model::transaction t(_workspace->getTransaction());

  typedef odb::query<CppAstNode>  AQuery;

  auto res (this->_workspace->getDb()->query<CppAstNode>(
    AQuery::symbolType == CppAstNode::SymbolType::Variable &&
    AQuery::astType == CppAstNode::AstType::Definition
  ));

  int cnt = 0;
  for(auto i(res.begin()); i != res.end(); ++i)
  {
      ++cnt;
  }
  EXPECT_EQ(6, cnt);
}

TEST_F(AstNodePropertyTests, FunctionVariableAstFlag)
{
  init("var_function_test.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/mangledname/");

  //session s;
  cc::model::transaction t(_workspace->getTransaction());

  typedef odb::query<CppAstNode>  AQuery;
  typedef odb::result<CppAstNode> AResult;

  int functionCallCnt = 0;
  AResult res ( this->_workspace->getDb()->query<CppAstNode>(
      AQuery::astType == CppAstNode::AstType::Usage &&
      AQuery::symbolType == CppAstNode::SymbolType::Function
  ));

  for(AResult::iterator i(res.begin()); i != res.end(); ++i)
    ++functionCallCnt;
  EXPECT_EQ(2, functionCallCnt);


  int variableRefCnt = 0;
  AResult res2 ( this->_workspace->getDb()->query<CppAstNode>(
      (AQuery::symbolType == CppAstNode::SymbolType::Variable) &&
      (AQuery::astType == CppAstNode::AstType::Read ||
      AQuery::astType == CppAstNode::AstType::Write)
  ));

  for(AResult::iterator i(res2.begin()); i != res2.end(); ++i)
  {
    ++variableRefCnt;
  }
  EXPECT_EQ(7, variableRefCnt);

  int writeCnt = 0;
  AResult res3 ( this->_workspace->getDb()->query<CppAstNode>(
      AQuery::astType == CppAstNode::AstType::Write
  ));

  for(AResult::iterator i(res3.begin()); i != res3.end(); ++i)
  {
    ++writeCnt;
  }
  EXPECT_EQ(1, writeCnt);


}

TEST_F(AstNodePropertyTests, TypeAndVarTest)
{
  init("typetest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/type/");

  cc::model::transaction t(_workspace->getTransaction());

  typedef odb::query<CppType>  CQuery;
  typedef odb::result<CppType> CResult;
  typedef odb::query<CppMemberType> MTQuery;

  CResult res ( this->_workspace->getDb()->query<CppType>(CQuery::name == "S") );

  CResult::iterator it(res.begin());
  
  EXPECT_EQ("S", it->name);

  auto fields_result = this->_workspace->getDb()->query<CppMemberType>(
    MTQuery::typeHash == it->mangledNameHash &&
    MTQuery::kind == CppMemberType::Kind::Field);

  auto functions_result = this->_workspace->getDb()->query<CppMemberType>(
    MTQuery::typeHash == it->mangledNameHash &&
    MTQuery::kind == CppMemberType::Kind::Method);

  std::vector<CppMemberType> fields =
    { fields_result.begin(), fields_result.end() };
  std::vector<CppMemberType> functions =
    { functions_result.begin(), functions_result.end() };
  
  EXPECT_EQ(13, fields.size());
  EXPECT_EQ(4, functions.size());


  typedef odb::query<CppVariable>  VQuery;
  typedef odb::result<CppVariable> VResult;

  VResult vres( this->_workspace->getDb()->query<CppVariable>(VQuery::name == "s") );
  VResult::iterator vi(vres.begin());
    
  EXPECT_EQ(it->mangledNameHash, vi->typeHash);
}
/*
TEST_F(AstNodePropertyTests, PrimitiveTypeTest)
{
  init("primitivetypetest.sqlite");
  this->_parser->parseProject(TOP_SRCDIR "parser/parser/test/test_src/type/");

  cc::model::transaction t(_workspace->getTransaction());

  typedef odb::query<CppVariable>  VQuery;
  typedef odb::result<CppVariable> VResult;

  typedef odb::query<CppType>  CQuery;
  typedef odb::result<CppType> CResult;


  {
    VResult res ( this->_workspace->getDb()->query<CppVariable>(VQuery::name == "b") );
    VResult::iterator it(res.begin());
    it->type.load();

    EXPECT_EQ("int", it->type->name);
  }

  {
    VResult res ( this->_workspace->getDb()->query<CppVariable>(VQuery::name == "a5") );
    VResult::iterator it(res.begin());
    it->type.load();

    EXPECT_EQ("unsigned long", it->type->name);
  }

}
*/

TEST_F(AstNodePropertyTests, FunctionTest)
{
  init("functiontest.sqlite");
  this->_parser.parseProject(TOP_SRCDIR "parser/parser/test/test_src/mangledname/");

  cc::model::transaction t(_workspace->getTransaction());

  typedef odb::query<CppFunction>  CQuery;
  typedef odb::result<CppFunction> CResult;


  CResult res ( this->_workspace->getDb()->query<CppFunction>( CQuery::qualifiedName == "f") );
  CResult::iterator i(res.begin());
  EXPECT_EQ(2, i->parameters.size());


  CResult res2( this->_workspace->getDb()->query<CppFunction>( CQuery::qualifiedName == "main") );
  CResult::iterator i2(res2.begin());
  EXPECT_EQ(3, i2->locals.size());
}

int main(int argc, char** argv) {
  StreamLog::setStrategy(std::shared_ptr<LogStrategy>(
      new StandardErrorLogStrategy()))  ;
  StreamLog::setLogLevel(INFO);
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
