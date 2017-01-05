// $Id$

#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <set>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>


#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <model/file.h>

#include "../../javaservice/src/javaservice.h"
#include "language-api/LanguageService.h"

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::service::language;
using namespace cc::service::core;
using namespace cc;

class Expected
{
public:
  typedef std::map<std::string, Expected> expects_t;

  explicit Expected() : _is_string(true) { }
  Expected(const Expected & lhs) : _is_string(lhs._is_string),
    _string(lhs._string),
    _expects(lhs._expects) { }
  Expected(const std::string & str) : _is_string(true), _string(str) { }
  Expected(const expects_t & exp) : _is_string(false), _expects(exp) { }

  Expected & operator=(const Expected & lhs)
  {
    if(&lhs!=this)
    {
      _is_string = lhs._is_string;
      _string = lhs._string;
      _expects = lhs._expects;
    }

    return *this;
  }

  Expected & operator=(const std::string & str)
  {
    _is_string = true;
    _string = str;
    _expects.clear();

    return *this;
  }

  Expected & operator=(const expects_t & exps)
  {
    _is_string = false;
    _string.clear();
    _expects = exps;

    return *this;
  }

  friend bool operator==(const Expected & rhs, const Expected & lhs)
  {
    return
      rhs._is_string == lhs._is_string &&
      rhs._string == lhs._string &&
      rhs._expects == lhs._expects;
  }

  bool isString() const { return _is_string; }

  const std::string & getString() const { return _string; }
  const expects_t & getExpects() const { return _expects; }

private:
  bool         _is_string;
  std::string  _string;
  expects_t    _expects;
};

class JavaServiceTest : public ::testing::Test
{
protected:
  std::shared_ptr<odb::database> _db;
  std::shared_ptr<JavaServiceHelper> _javaserviceHelper;
  std::shared_ptr<JavaServiceHandler> _javaservice;

  JavaServiceTest() : _javaserviceHelper(nullptr), _javaservice(nullptr) {}

  virtual void TearDown()
  {
    _javaservice.reset();
    _javaserviceHelper.reset();
    _db.reset();
  }

  void init(const char* dbname)
  {
    std::string connStr = dbname;
    _db = util::createDatabase(connStr);
    _javaserviceHelper.reset( new JavaServiceHelper(_db) );
    _javaservice.reset(new JavaServiceHandler(*_javaserviceHelper));
  }

  AstNodeInfo getAstNodeInfoByPos(int line, int col, model::FileId fid_)
  {
    FileId fid;
    fid.__set_fid(std::to_string(fid_));

    Position pos;
    pos.__set_line(line);
    pos.__set_column(col);

    FilePosition fp;
    fp.__set_file(fid);
    fp.__set_pos(pos);

    return  _javaserviceHelper->getAstNodeInfoByPosition(fp, std::vector<std::string>());
  }

  model::FileId getFileId(const std::string& fn)
  {
    odb::transaction t(_db->begin());

    typedef odb::query<model::File> FQuery;

    auto res (_db->query<model::File>(
      FQuery::filename == fn
    ));

    return (*res.begin()).id;
  }

  void checkDefinition(int line, int col, model::FileId fid, int expected_line, model::FileId expected_fid)
  {
    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> results;
    _javaservice->getReferences(results, anFrom.astNodeId, RefTypes::GetDef);
    //AstNodeInfo anTo = _javaservice->getDefinition(anFrom.astNodeId);
    EXPECT_EQ(1u, results.size());
    if(1u != results.size()) { return; }
    EXPECT_EQ(expected_line, results[0].range.range.startpos.line);
    EXPECT_EQ(expected_fid, stoull(results[0].range.file.fid));
  }

  void checkInfoTree(AstNodeInfo & astnodeinfo, const std::vector<InfoNode> & subinfonodes,
    const Expected::expects_t & exps, const unsigned int catlevel=0)
  {
    std::size_t hitcounter = 0;
    std::set<std::string> visited;

    for(const InfoNode & subnode: subinfonodes)
    {
      Expected::expects_t::const_iterator it = exps.find(subnode.label);
      bool in_cat = false;

      if(exps.end()==it && catlevel<subnode.category.size())
      {
        it = exps.find(subnode.category[catlevel]);
        in_cat = true;
      }

      if(exps.end()!=it)
      {
        const Expected & exp = it->second;
        if(in_cat && exp.isString()) { continue; }
        if(!in_cat && !exp.isString()) { continue; }

        if(in_cat)
        {
          if(0<visited.count(it->first)) { continue; }
          visited.insert(it->first);
        }

        ++hitcounter;

        if(exp.isString())
        {
          EXPECT_EQ(exp.getString(), subnode.value);
        }
        else
        {
          // TODO: this solution is a little fragile, the actual categories should be passed to checkInfoTree()
          if(catlevel+1<subnode.category.size())
          {
            checkInfoTree(astnodeinfo, subinfonodes, exp.getExpects(), catlevel+1);
          }
          else
          if(0!=subnode.query.queryId)
          {
            std::vector<InfoNode> subsubinfonodes;
            _javaservice->getSubInfoTree(subsubinfonodes, astnodeinfo.astNodeId, subnode.query);
            checkInfoTree(astnodeinfo, subsubinfonodes, exp.getExpects());
          }
          else
          {
            checkInfoTree(astnodeinfo, subinfonodes, exp.getExpects());
          }
        }
      }
    }

    std::stringstream unvisited;
    for(const auto & p : exps)
    {
      if(!visited.count(p.first)) { unvisited << p.first << ", "; }
    }

    EXPECT_EQ(hitcounter, exps.size()) << unvisited.str();
  }

  void checkInfoTree(int line, int col, model::FileId fid, const Expected::expects_t & exps)
  {
    AstNodeInfo astnodeinfo = getAstNodeInfoByPos(line, col, fid);

    std::vector<InfoNode> infonodes;
    _javaservice->getInfoTree(infonodes, astnodeinfo.astNodeId);
    checkInfoTree(astnodeinfo, infonodes, exps);
  }

  void checkReferences(int line, int col, model::FileId fid, const std::set<int>& expected)
  {
    FileId fileId;
    fileId.__set_fid(std::to_string(fid));

    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> refs;
    _javaservice->getReferencesInFile(refs, anFrom.astNodeId, RefTypes::GetUsage, fileId);

    checkSets(expected, refs);
  }

  void checkCalledFunctions(model::FileId fid, int line, int col, std::vector<std::pair<model::FileId, int>> expected)
  {
    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> refs;
    _javaservice->getReferences(refs, anFrom.astNodeId, RefTypes::GetFuncCalls);

    checkResult(expected, refs);
  }

  void checkCallerFunctions(int line, int col, model::FileId fid, model::FileId searchIn, const std::set<int>& expected)
  {
    FileId fileId;
    fileId.__set_fid(std::to_string(searchIn));

    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> refs = _javaserviceHelper->getCallerFunctions(anFrom.astNodeId, fileId);

    checkSets(expected, refs);
  }

  void checkSets(const std::set<int>& expected, const std::vector<AstNodeInfo>& got)
  {
    // A Sort would be nice on got
    EXPECT_EQ(expected.size(), got.size());

    for(const auto& ref : got)
    {
      EXPECT_EQ(1u, expected.count(ref.range.range.startpos.line));
    }
  }

  void checkResult(std::vector<std::pair<model::FileId, int>>& expected, std::vector<AstNodeInfo>& got)
  {
    EXPECT_EQ(expected.size(), got.size());

    std::sort(expected.begin(), expected.end());
    std::sort(got.begin(), got.end(), [](const AstNodeInfo & lhs, const AstNodeInfo & rhs){
      return (lhs.range.file.fid<rhs.range.file.fid) ||
        (lhs.range.file.fid==rhs.range.file.fid &&
          lhs.range.range.startpos.line<rhs.range.range.startpos.line);
    });

    for(std::size_t i=0,n=expected.size();i!=n;++i)
    {
      EXPECT_EQ(expected[i].first, std::stoull(got[i].range.file.fid));
      EXPECT_EQ(expected[i].second, got[i].range.range.startpos.line);
    }
  }
};

TEST_F(JavaServiceTest, getDefinition_Simple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simplejava/simplejava.sqlite");
  model::FileId fileFoo = getFileId("Foo.java");
  model::FileId fileBar = getFileId("Bar.java");

  checkDefinition(18, 13, fileFoo, 5, fileFoo); // bar
  checkDefinition(28, 7, fileBar, 9, fileBar); // logic
  checkDefinition(41, 27, fileBar, 7, fileFoo); // Foo.VALUE
}

TEST_F(JavaServiceTest, getCallerFunctions_Simple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simplejava/simplejava.sqlite");
  model::FileId fileBar = getFileId("Bar.java");
  model::FileId fileFoo = getFileId("Foo.java");

  checkCallerFunctions(21, 12, fileBar, fileFoo, {18}); // Bar.bar()
  checkCallerFunctions(26, 9, fileBar, fileFoo, {12}); // Bar.bar(boolean)
}

TEST_F(JavaServiceTest, getCalledFunctions_Simple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simplejava/simplejava.sqlite");
  model::FileId fileFoo = getFileId("Foo.java");

  checkCalledFunctions(fileFoo, 9, 11, {{fileFoo, 11}, {fileFoo, 12}}); // Foo.Foo()
  checkCalledFunctions(fileFoo, 15, 13, {{fileFoo, 18}}); // Foo.foo()
}

TEST_F(JavaServiceTest, getDefinition_JavaTest)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javatest/javatest.sqlite");
  model::FileId fileDeriv = getFileId("Deriv.java");
  model::FileId fileIface = getFileId("Iface.java");
  model::FileId fileMain = getFileId("Main.java");
  model::FileId fileBase = getFileId("Base.java");

  checkDefinition(3, 19, fileMain, 5, fileBase); // import Base
  checkDefinition(4, 19, fileMain, 5, fileDeriv); // import Deriv
  checkDefinition(18, 6, fileMain, 8, fileMain); // MyEnum
  checkDefinition(18, 26, fileMain, 10, fileMain); // MyEnum.Value1
  checkDefinition(36, 14, fileMain, 32, fileMain); // args
  checkDefinition(40, 16, fileMain, 11, fileDeriv); // new Deriv
  checkDefinition(40, 22, fileMain, 5, fileDeriv); // Deriv(String)
  checkDefinition(44, 9, fileMain, 5, fileBase); // Base
  checkDefinition(54, 8, fileMain, 16, fileMain); // enumTest

  checkDefinition(13, 5, fileDeriv, 20, fileBase); // super(int)
  checkDefinition(14, 5, fileDeriv, 7, fileDeriv); // str
  checkDefinition(15, 5, fileDeriv, 9, fileDeriv); // something

  checkDefinition(5, 19, fileIface, 5, fileIface); // foo2
}

TEST_F(JavaServiceTest, getCalledFunctions_JavaTest)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javatest/javatest.sqlite");
  model::FileId fileDeriv = getFileId("Deriv.java");

  checkCalledFunctions(fileDeriv, 11, 11, {{fileDeriv, 13}}); // Deriv.Deriv(String)
}

TEST_F(JavaServiceTest, getInfoTree_Simple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simplejava/simplejava.sqlite");
  model::FileId fileFoo = getFileId("Foo.java");
  model::FileId fileBar = getFileId("Bar.java");

  Expected::expects_t expFoo;
  expFoo["Name"] = "Foo";
  expFoo["Qualified Name"] = "two.Foo";
  expFoo["Defined"] = "Foo.java:3:8";
  expFoo["Super Class"] = "Object";
  expFoo["Interfaces"] = Expected::expects_t();
  expFoo["Inherits From"] = Expected::expects_t({ {"Object", Expected("(external)")} });
  expFoo["Inherited By"] = Expected::expects_t();
  expFoo["Methods"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"Foo()", Expected(" ")},
      })
    },
    {
      "Package", Expected::expects_t(
      {
        {"foo()", Expected("boolean")}
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });
  expFoo["Members"] = Expected::expects_t(
  {
    {
      "Package", Expected::expects_t(
      {
        {"bar", Expected("Bar")},
        {"VALUE", Expected("int")}
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });

  Expected::expects_t expBar;
  expBar["Name"] = "Bar";
  expBar["Qualified Name"] = "two.Bar";
  expBar["Defined"] = "Bar.java:6:8";
  expBar["Super Class"] = "Object";
  expBar["Interfaces"] = Expected::expects_t();
  expBar["Inherits From"] = Expected::expects_t({ {"Object", Expected("(external)")} });
  expBar["Inherited By"] = Expected::expects_t();
  expBar["Methods"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"Bar()", Expected(" ")},
        {"Bar(boolean)", Expected(" ")},
        {"setLogic(boolean)", Expected("void")},
        {"testFooVALUE(int)", Expected("boolean")},
      })
    },
    {
      "Package", Expected::expects_t(
      {
        {"bar()", Expected("boolean")},
        {"bar(boolean)", Expected("void")}
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });
  expBar["Members"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"shared", Expected("int")},
      })
    },
    {
      "Private", Expected::expects_t(
      {
        {"staticLogic", Expected("boolean")},
      })
    },
    {
      "Package", Expected::expects_t(
      {
        {"logic", Expected("boolean")},
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });

  checkInfoTree(3, 15, fileFoo, expFoo);
  checkInfoTree(6, 15, fileBar, expBar);
}

TEST_F(JavaServiceTest, getInfoTree_ClassUsage_Simple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simplejava/simplejava.sqlite");
  model::FileId fileBar = getFileId("Bar.java");

  Expected::expects_t expBar;
  expBar["Name"] = "Bar";
  expBar["Usage"] = Expected::expects_t(
  {
    {
      "Local", Expected::expects_t()
    },
    {
      "Field", Expected::expects_t(
      {
        {
          "Foo.java (1)", Expected::expects_t(
          {
            {"5:7", Expected("  Bar bar = new Bar();")}
          })
        }
      })
    },
    {
      "Parameter", Expected::expects_t()
    },
    {
      "Return type", Expected::expects_t()
    }
  });

  checkInfoTree(6, 15, fileBar, expBar);
}

TEST_F(JavaServiceTest, getDefinition_test_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");

  checkDefinition(8, 13, fileUsage, 19, fileUsage);       // testABC()
  checkDefinition(9, 13, fileUsage, 34, fileUsage);       // testAbstract()
  checkDefinition(10, 13, fileUsage, 54, fileUsage);      // testDeepCallChain()
  checkDefinition(11, 13, fileUsage, 58, fileUsage);      // testMethodsAndConstructors()
  checkDefinition(12, 13, fileUsage, 86, fileUsage);      // testVariables()
  checkDefinition(13, 13, fileUsage, 98, fileUsage);      // testAnonNestedMultiClass()
  checkDefinition(14, 13, fileUsage, 104, fileUsage);     // testGenericClass()
}

TEST_F(JavaServiceTest, getDefinition_testABC_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileClassA      = getFileId("ClassA.java");
  model::FileId fileClassB      = getFileId("ClassB.java");
  model::FileId fileClassC      = getFileId("ClassC.java");

  checkDefinition(20, 31, fileUsage, 3, fileClassA);      // ClassA
  checkDefinition(21, 14, fileUsage, 3, fileClassB);      // ClassB
  checkDefinition(22, 14, fileUsage, 3, fileClassC);      // ClassC
  checkDefinition(23, 14, fileUsage, 3, fileClassA);      // ClassB
  checkDefinition(24, 14, fileUsage, 3, fileClassA);      // ClassC
  checkDefinition(20, 25, fileUsage, 7, fileClassA);      // ClassA(int x)
  checkDefinition(21, 25, fileUsage, 5, fileClassB);      // ClassB(int y)
  checkDefinition(22, 25, fileUsage, 5, fileClassC);      // ClassC()
  checkDefinition(23, 25, fileUsage, 5, fileClassB);      // ClassB(int y)
  checkDefinition(24, 25, fileUsage, 5, fileClassC);      // ClassC()
  checkDefinition(26, 22, fileUsage, 11, fileClassA);     // function()
  checkDefinition(27, 22, fileUsage, 10, fileClassB);     // funcion()
  checkDefinition(28, 22, fileUsage, 14, fileClassB);     // testParentAndChildFunction()
  checkDefinition(29, 22, fileUsage, 10, fileClassC);     // function()
  checkDefinition(30, 22, fileUsage, 11, fileClassA);     // function()
  checkDefinition(31, 22, fileUsage, 11, fileClassA);     // function()
}

TEST_F(JavaServiceTest, getDefinition_testAbstract_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileAbs         = getFileId("AbstractClass.java");
  model::FileId fileCEAI        = getFileId("ClassExtendsAndImplements.java");
  model::FileId fileInterfaceA  = getFileId("InterfaceA.java");
  model::FileId fileInterfaceB  = getFileId("InterfaceB.java");

  checkDefinition(35, 17, fileUsage, 3, fileAbs);         // AbstractClass
  checkDefinition(36, 21, fileUsage, 7, fileAbs);         // abstractClassMethod()
  checkDefinition(38, 22, fileUsage, 3, fileCEAI);        // CEAI
  checkDefinition(39, 22, fileUsage, 17, fileCEAI);       // abstractClassMethod()
  checkDefinition(40, 22, fileUsage, 7, fileCEAI);        // interfaceAMethod()
  checkDefinition(41, 22, fileUsage, 12, fileCEAI);       // interfaceBMethod()
  checkDefinition(42, 32, fileUsage, 5, fileInterfaceA);  // interfaceAInt
  checkDefinition(43, 32, fileUsage, 5, fileInterfaceB);  // interfaceBInt
}

TEST_F(JavaServiceTest, getDefinition_testDeepCallChain_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileFCIOC_1     = getFileId("FunctionCallInOtherClass_1.java");

  checkDefinition(55, 20, fileUsage, 3, fileFCIOC_1);     // FunctionCallInOtherClass_1
  checkDefinition(55, 38, fileUsage, 5, fileFCIOC_1);     // step1()
}

TEST_F(JavaServiceTest, getDefinition_testMethodsAndConstructors_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileMAC         = getFileId("MethodsAndConstructors.java");

  checkDefinition(59, 18, fileUsage, 3, fileMAC);         // MethodsAndConstructors
  checkDefinition(59, 41, fileUsage, 6, fileMAC);         // MethodsAndConstructors()
  checkDefinition(60, 41, fileUsage, 9, fileMAC);         // MethodsAndConstructors(int)
  checkDefinition(61, 41, fileUsage, 12, fileMAC);        // MethodsAndConstructors(double)
  checkDefinition(62, 41, fileUsage, 15, fileMAC);        // MethodsAndConstructors(int,int)
  checkDefinition(63, 41, fileUsage, 18, fileMAC);        // MethodsAndConstructors(double,double)
  checkDefinition(64, 41, fileUsage, 21, fileMAC);        // MethodsAndConstructors(int,double)
  checkDefinition(65, 41, fileUsage, 24, fileMAC);        // MethodsAndConstructors(double,int)
  checkDefinition(67, 10, fileUsage, 59, fileUsage);      // localVariable: mac1
  checkDefinition(67, 22, fileUsage, 42, fileMAC);        // recursionTimes30()
  checkDefinition(68, 22, fileUsage, 50, fileMAC);        // callOtherFunctionStopAt30_A(int)
  checkDefinition(70, 33, fileUsage, 86, fileMAC);        // staticMethodReturnsInt()
  checkDefinition(71, 33, fileUsage, 90, fileMAC);        // staticMethodReturnsString()
  checkDefinition(72, 33, fileUsage, 79, fileMAC);        // methodReturnsString()
  checkDefinition(73, 33, fileUsage, 75, fileMAC);        // methodReturnsInt()
  checkDefinition(74, 19, fileUsage, 83, fileMAC);        // staticMethodReturnsVoid()
  checkDefinition(75, 19, fileUsage, 72, fileMAC);        // methodReturnsVoid()
  checkDefinition(77, 20, fileUsage, 95, fileMAC);        // overloadedMethod()
  checkDefinition(78, 20, fileUsage, 98, fileMAC);        // overloadedMethod(int)
  checkDefinition(79, 20, fileUsage, 101, fileMAC);       // overloadedMethod(int,int)
  checkDefinition(80, 20, fileUsage, 104, fileMAC);       // overloadedMethod(double)
  checkDefinition(81, 20, fileUsage, 107, fileMAC);       // overloadedMethod(double,double)
  checkDefinition(82, 20, fileUsage, 110, fileMAC);       // overloadedMethod(int,double)
  checkDefinition(83, 20, fileUsage, 113, fileMAC);       // overloadedMethod(double,int)
}

TEST_F(JavaServiceTest, getDefinition_testVariables_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileVariables   = getFileId("Variables.java");

  checkDefinition(87, 14, fileUsage, 13, fileVariables);  // Variables
  checkDefinition(89, 28, fileUsage, 20, fileVariables);  // serializablePublicInt
  checkDefinition(90, 25, fileUsage, 22, fileVariables);  // publicInt
  checkDefinition(91, 25, fileUsage, 27, fileVariables);  // publicStaticInt
  checkDefinition(92, 25, fileUsage, 42, fileVariables);  // publicString
  checkDefinition(93, 25, fileUsage, 47, fileVariables);  // publicStaticString
  checkDefinition(94, 25, fileUsage, 62, fileVariables);  // publicObject
  checkDefinition(95, 25, fileUsage, 67, fileVariables);  // publicStaticObject
}

TEST_F(JavaServiceTest, getDefinition_testAnonNestedMultiClass_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");
  model::FileId fileANMC        = getFileId("AnonNestedMultiClass.java");

  checkDefinition(99, 18, fileUsage, 3, fileANMC);        // AnonNestedMultiClass
  checkDefinition(99, 39, fileUsage, 7, fileANMC);        // AnonNestedMultiClass()
  checkDefinition(101, 22, fileUsage, 5, fileANMC);       // globalVariableInt
}

TEST_F(JavaServiceTest, getDefinition_testLocalVariables_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage       = getFileId("Usage.java");

  checkDefinition(116, 11, fileUsage, 115, fileUsage);    // ceai
  checkDefinition(119, 11, fileUsage, 118, fileUsage);    // anmc
  checkDefinition(127, 35, fileUsage, 122, fileUsage);    // localInt1
  checkDefinition(128, 35, fileUsage, 124, fileUsage);    // localDouble1
  checkDefinition(129, 35, fileUsage, 122, fileUsage);    // localInt1
  checkDefinition(129, 45, fileUsage, 123, fileUsage);    // localInt2
  checkDefinition(130, 35, fileUsage, 124, fileUsage);    // localDouble1
  checkDefinition(130, 48, fileUsage, 125, fileUsage);    // localDouble2
  checkDefinition(131, 35, fileUsage, 124, fileUsage);    // localDouble1
  checkDefinition(131, 48, fileUsage, 122, fileUsage);    // localInt1
  checkDefinition(132, 35, fileUsage, 122, fileUsage);    // localInt1
  checkDefinition(132, 45, fileUsage, 124, fileUsage);    // localDouble1
}

/*
 * Generics not yet supported (2014.12.14.)
 *
 * TEST_F(JavaServiceTest, getDefinition_testGenericClass_JavaDemo)
 * {
 *   init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
 *   model::FileId fileUsage       = getFileId("Usage.java");
 *   model::FileId fileGClass      = getFileId("GenericClass.java");
 *
 *   checkDefinition(105, 16, fileUsage, 6, fileGClass);     // GenericClass
 *   checkDefinition(107, 20, fileUsage, 10, fileGClass);    // set(T)
 *   checkDefinition(108, 44, fileUsage, 14, fileGClass);    // get()
 *   checkDefinition(110, 26, fileUsage, 18, fileGClass);    // simpleGenericFunction()
 *   checkDefinition(111, 26, fileUsage, 22, fileGClass);    // genericFunction()
 * }
 *
 */

TEST_F(JavaServiceTest, getCallerFunctions_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage   = getFileId("Usage.java");
  model::FileId fileFCIOC_1 = getFileId("FunctionCallInOtherClass_1.java");
  model::FileId fileClassA  = getFileId("ClassA.java");
  model::FileId fileMAC     = getFileId("MethodsAndConstructors.java");

  checkCallerFunctions(19, 28, fileUsage, fileUsage, {8});            // testABC()
  checkCallerFunctions(34, 28, fileUsage, fileUsage, {9});            // testAbstract()
  checkCallerFunctions(54, 28, fileUsage, fileUsage, {10});           // testDeepCallChain()
  checkCallerFunctions(58, 28, fileUsage, fileUsage, {11});           // testMethodsAndConstructors()
  checkCallerFunctions(86, 28, fileUsage, fileUsage, {12});           // testVariables()
  checkCallerFunctions(98, 28, fileUsage, fileUsage, {13});           // testAnonNestedMultiClass()
  checkCallerFunctions(104, 28, fileUsage, fileUsage, {14});          // testGenericClass()
  checkCallerFunctions(5, 27, fileFCIOC_1, fileUsage, {55});          // step1()
  checkCallerFunctions(11, 21, fileClassA, fileUsage, {26, 30, 31});  // classA.function()
  checkCallerFunctions(6, 22, fileMAC, fileUsage, {59});              // MethodsAndConstructors()
  checkCallerFunctions(98, 25, fileMAC, fileUsage, {78, 127});        // overloadedMethod(int)
}

TEST_F(JavaServiceTest, getCalledFunctions_JavaDemo)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage = getFileId("Usage.java");

  checkCalledFunctions(fileUsage, 7, 26, { {fileUsage, 8}, {fileUsage, 9}, {fileUsage, 10}, {fileUsage, 11},       // Usage.test()
                                           {fileUsage, 12}, {fileUsage, 13}, {fileUsage, 14}
                                         });
  checkCalledFunctions(fileUsage, 19, 28, { {fileUsage, 20}, {fileUsage, 21}, {fileUsage, 23}, {fileUsage, 22},    // Usage.testABC()
                                            {fileUsage, 24}, {fileUsage, 26}, {fileUsage, 27}, {fileUsage, 28},
                                            {fileUsage, 29}, {fileUsage, 30}, {fileUsage, 31}
                                          });
  checkCalledFunctions(fileUsage, 34, 28, { {fileUsage, 35}, {fileUsage, 36}, {fileUsage, 38}, {fileUsage, 39},    // Usage.testAbstract()
                                            {fileUsage, 40}, {fileUsage, 41}, {fileUsage, 45}, {fileUsage, 46},
                                            {fileUsage, 49}, {fileUsage, 50}
                                          });
}

TEST_F(JavaServiceTest, getInfoTree_JavaDemo_Classes)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage = getFileId("Usage.java");
  model::FileId fileClassA = getFileId("ClassA.java");
  model::FileId fileClassB = getFileId("ClassB.java");


  Expected::expects_t expUsage;
  expUsage["Name"] = "Usage";
  expUsage["Qualified Name"] = "javademo.Usage";
  expUsage["Defined"] = "Usage.java:5:8";
  expUsage["Super Class"] = "Object";
  expUsage["Interfaces"] = Expected::expects_t();
  expUsage["Inherits From"] = Expected::expects_t({ {"Object", Expected("(external)")} });
  expUsage["Inherited By"] = Expected::expects_t();
  expUsage["Methods"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"test()", Expected("void")},
        {"testABC()", Expected("void")},
        {"testAbstract()", Expected("void")},
        {"testDeepCallChain()", Expected("void")},
        {"testMethodsAndConstructors()", Expected("void")},
        {"testVariables()", Expected("void")},
        {"testAnonNestedMultiClass()", Expected("void")},
        {"testGenericClass()", Expected("void")},
        {"testLocalVariables()", Expected("void")}
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });
  expUsage["Members"] = Expected::expects_t();


  Expected::expects_t expClassA;
  expClassA["Name"] = "ClassA";
  expClassA["Qualified Name"] = "javademo.ClassA";
  expClassA["Defined"] = "ClassA.java:3:8";
  expClassA["Super Class"] = "Object";
  expClassA["Interfaces"] = Expected::expects_t();
  expClassA["Inherits From"] = Expected::expects_t({ {"Object", Expected("(external)")} });
  expClassA["Inherited By"] = Expected::expects_t({ {"ClassB", Expected("ClassB.java:3:8")} });
  expClassA["Methods"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"ClassA(int)", Expected(" ")},
        {"function()", Expected("void")}
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });
  expClassA["Members"] = Expected::expects_t(
  {
    {
      "Private", Expected::expects_t(
      {
        {"x", Expected("int")},
      })
    },
    {
      "Inherited", Expected::expects_t()
    }
  });
  expClassA["Usage"] = Expected::expects_t(
  {
    {
      "Local", Expected::expects_t({
        {
          "Usage.java (3)", Expected::expects_t(
          {
            {"20:16", Expected("        ClassA classA = new ClassA(2);")},
            {"23:16", Expected("        ClassA classX = new ClassB(4);")},
            {"24:16", Expected("        ClassA classY = new ClassC();")}
          })
        }
      })
    },
    {
      "Field", Expected::expects_t()
    },
    {
      "Parameter", Expected::expects_t()
    },
    {
      "Return type", Expected::expects_t()
    }
  });


  Expected::expects_t expClassB;
  expClassB["Name"] = "ClassB";
  expClassB["Qualified Name"] = "javademo.ClassB";
  expClassB["Defined"] = "ClassB.java:3:8";
  expClassB["Super Class"] = "ClassA";
  expClassB["Interfaces"] = Expected::expects_t();
  expClassB["Inherits From"] = Expected::expects_t({ {"ClassA", Expected("ClassA.java:3:8")} });
  expClassB["Inherited By"] = Expected::expects_t({ {"ClassC", Expected("ClassC.java:3:8")} });
  expClassB["Methods"] = Expected::expects_t(
  {
    {
      "Public", Expected::expects_t(
      {
        {"ClassB(int)", Expected(" ")},
        {"function()", Expected("void")},
        {"testParentAndChildFunction()", Expected("void")}
      })
    },
    {
      "Inherited", Expected::expects_t({
        {
          "Public", Expected::expects_t(
          {
            {"ClassA(int)", Expected(" ")},
            {"function()", Expected("void")}
          })
        }
      })
    }
  });
  expClassB["Members"] = Expected::expects_t(
  {
    {
      "Inherited", Expected::expects_t({
        {
          "Private", Expected::expects_t(
          {
            {"x", Expected("int")},
          })
        }
      })
    }
  });
  expClassB["Usage"] = Expected::expects_t(
  {
    {
      "Local", Expected::expects_t({
        {
          "Usage.java (1)", Expected::expects_t(
          {
            {"21:16", Expected("        ClassB classB = new ClassB(3);")}
          })
        }
      })
    },
    {
      "Field", Expected::expects_t()
    },
    {
      "Parameter", Expected::expects_t()
    },
    {
      "Return type", Expected::expects_t()
    }
  });


  checkInfoTree(5, 16, fileUsage, expUsage);
  checkInfoTree(3, 16, fileClassA, expClassA);
  checkInfoTree(3, 16, fileClassB, expClassB);
}

TEST_F(JavaServiceTest, getInfoTree_JavaDemo_Variables)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage = getFileId("Usage.java");


  Expected::expects_t expVariableCeai;
  expVariableCeai["Name"] = "ceai";
  expVariableCeai["Qualified Name"] = "javademo.Usage.testAbstract()._1.ceai";
  expVariableCeai["Type"] = "javademo.ClassExtendsAndImplements";
  expVariableCeai["Declaration"] = "Usage.java:38:35";
  expVariableCeai["Reads"] = Expected::expects_t(
  {
    {
      "Usage.java (5)", Expected::expects_t(
      {
        {"39:9", Expected("        ceai.abstractClassMethod();")},
        {"40:9", Expected("        ceai.interfaceAMethod();")},
        {"41:9", Expected("        ceai.interfaceBMethod();")},
        {"42:18", Expected("        int x1 = ceai.interfaceAInt;")},
        {"43:18", Expected("        int x2 = ceai.interfaceBInt;")}
      })
    }
  });
  expVariableCeai["Writes"] = Expected::expects_t(
  {
    {
      "Usage.java (1)", Expected::expects_t(
      {
        {"38:35", Expected("        ClassExtendsAndImplements ceai = new ClassExtendsAndImplements();")}
      })
    }
  });


  Expected::expects_t expVariableMac;
  expVariableMac["Name"] = "mac";
  expVariableMac["Qualified Name"] = "javademo.Usage.testLocalVariables()._1.mac";
  expVariableMac["Type"] = "javademo.MethodsAndConstructors";
  expVariableMac["Declaration"] = "Usage.java:121:32";
  expVariableMac["Reads"] = Expected::expects_t(
  {
    {
      "Usage.java (9)", Expected::expects_t(
      {
        {"127:9", Expected("        mac.overloadedMethod(localInt1);")},
        {"128:9", Expected("        mac.overloadedMethod(localDouble1);")},
        {"129:9", Expected("        mac.overloadedMethod(localInt1,localInt2);")},
        {"130:9", Expected("        mac.overloadedMethod(localDouble1,localDouble2);")},
        {"131:9", Expected("        mac.overloadedMethod(localDouble1,localInt1);")},
        {"132:9", Expected("        mac.overloadedMethod(localInt1,localDouble1);")},
        {"136:42", Expected("        mac = new MethodsAndConstructors(mac);")},
        {"137:42", Expected("        mac = new MethodsAndConstructors(mac,mac);")},
        {"137:46", Expected("        mac = new MethodsAndConstructors(mac,mac);")}
      })
    }
  });
  expVariableMac["Writes"] = Expected::expects_t(
  {
    {
      "Usage.java (5)", Expected::expects_t(
      {
        {"121:32", Expected("        MethodsAndConstructors mac = new MethodsAndConstructors(10,2);")},
        {"134:9", Expected("        mac = null;")},
        {"135:9", Expected("        mac = new MethodsAndConstructors(1.0);")},
        {"136:9", Expected("        mac = new MethodsAndConstructors(mac);")},
        {"137:9", Expected("        mac = new MethodsAndConstructors(mac,mac);")}
      })
    }
  });


  checkInfoTree(38, 37, fileUsage, expVariableCeai);
  checkInfoTree(127, 10, fileUsage, expVariableMac);
}


TEST_F(JavaServiceTest, getInfoTree_JavaDemo_Functions)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/javademo/javademo.sqlite");
  model::FileId fileUsage = getFileId("Usage.java");


  Expected::expects_t expFunctionTestParentAndChildFunction;
  expFunctionTestParentAndChildFunction["Name"] = "testParentAndChildFunction";
  expFunctionTestParentAndChildFunction["Qualified Name"] = "javademo.ClassB.testParentAndChildFunction";
  expFunctionTestParentAndChildFunction["Signature"] = "void javademo.ClassB.testParentAndChildFunction()";
  expFunctionTestParentAndChildFunction["Return type"] = "void";
  expFunctionTestParentAndChildFunction["Declaration"] = "ClassB.java:14:12";
  expFunctionTestParentAndChildFunction["Parameters"] = Expected::expects_t();
  expFunctionTestParentAndChildFunction["Local Variables"] = Expected::expects_t();
  expFunctionTestParentAndChildFunction["Calls"] = Expected::expects_t({
    {
      "javademo.ClassB.function()", Expected::expects_t(
      {
        {"16:9", Expected("this.function()")},
        {"19:9", Expected("(new ClassB(2)).function()")}
      })
    }
    ,
    {
      "javademo.ClassA.function()", Expected::expects_t(
      {
        {"18:9", Expected("(new ClassA(1)).function()")}
      })
    }
    ,
    {
      "javademo.ClassA.ClassA(int)", Expected::expects_t(
      {
        {"18:10", Expected("new ClassA(1)")}
      })
    }
    ,
    {
      "javademo.ClassB.ClassB(int)", Expected::expects_t(
      {
        {"19:10", Expected("new ClassB(2)")}
      })
    }
    ,
    {
      "javademo.ClassC.function()", Expected::expects_t(
      {
        {"20:9", Expected("(new ClassC()).function()")}
      })
    }
    ,
    {
      "javademo.ClassC.ClassC()", Expected::expects_t({
      {
        {"20:10", Expected("new ClassC()")}
      }
      })
    }
  });
  expFunctionTestParentAndChildFunction["Called by"] = Expected::expects_t({
    {
      "Usage.java (1)", Expected::expects_t({
        {
          "testABC()", Expected::expects_t({
            {
              "Called by", Expected::expects_t({
                {
                  "Usage.java (1)", Expected::expects_t({
                    {
                      "test()", Expected::expects_t({
                        {"Called by", Expected::expects_t()}
                      })
                    }
                    ,
                    {"8:9", Expected("testABC()")}
                  })
                }
              })
            }
          })
        }
        ,
        {"28:9", Expected("classB.testParentAndChildFunction()")}
      })
    }
  });


  Expected::expects_t expFunctionSimpleFunctionTest;
  expFunctionSimpleFunctionTest["Name"] = "simpleFunctionTest";
  expFunctionSimpleFunctionTest["Qualified Name"] = "javademo.Usage.simpleFunctionTest";
  expFunctionSimpleFunctionTest["Signature"] = "int javademo.Usage.simpleFunctionTest(int,double,javademo.Usage)";
  expFunctionSimpleFunctionTest["Return type"] = "int";
  expFunctionSimpleFunctionTest["Declaration"] = "Usage.java:140:19";
  expFunctionSimpleFunctionTest["Annotations"] = Expected::expects_t();
  expFunctionSimpleFunctionTest["Parameters"] = Expected::expects_t({
    {"intParam", Expected("int")},
    {"doubleParam", Expected("double")},
    {"usageParam", Expected("javademo.Usage")}
  });
  expFunctionSimpleFunctionTest["Local Variables"] = Expected::expects_t({
    {"localVariableDouble", Expected("double")},
    {"localVariableInt", Expected("int")},
    {"localVariableUsage", Expected("javademo.Usage")}
  });
  expFunctionSimpleFunctionTest["Calls"] = Expected::expects_t();
  expFunctionSimpleFunctionTest["Called by"] = Expected::expects_t({
    {
      "Usage.java (1)", Expected::expects_t({
        {
          "iCallSimpleFunctionTest()", Expected::expects_t({
            {
              "Called by", Expected::expects_t()
            }
          })
        }
        ,
        {"150:9", Expected("simpleFunctionTest(1, 1.0, usage)")}
      })
    }
  });


  checkInfoTree(28, 26, fileUsage, expFunctionTestParentAndChildFunction);
  checkInfoTree(150, 15, fileUsage, expFunctionSimpleFunctionTest);
}


int main(int argc, char** argv)
{
  util::StreamLog::setStrategy(std::shared_ptr<util::LogStrategy>(
    new util::StandardErrorLogStrategy()));
  util::StreamLog::setLogLevel(util::INFO);
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
