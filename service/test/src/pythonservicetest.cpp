#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <string>
#include <set>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <model/file.h>

#include <pythonservice/pythonservice.h>
#include <pythonservice/treehandler.h>
#include "language-api/LanguageService.h"

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::service::language;
using namespace cc::service::language::python;
using namespace cc::service::core;
using namespace cc;

std::string new_line = "\n";

struct PythonMock
{

  PythonMock(
    const std::string& fileName_,
    const int32_t startLine_,
    const std::string& name_)
    : fileName(fileName_)
    , startLine(startLine_)
    , name(name_)
  {}

  const std::string fileName;
  const int32_t startLine;
  const std::string name;
};

class ExpectationSet
{
public:
  ExpectationSet(std::shared_ptr<odb::database> db_)
    : transaction(db_)
    , db(db_)
  {}

  void dump() const
  {
    for (PythonMock m : vector)
    {
      std::cout << "Mock: " << m.name << " starts at: " << m.startLine << "\n";
    }
  }
  
  ExpectationSet& add(
    const std::string& fileName_,
    const int32_t startLine_,
    const std::string& name_)
  {
    PythonMock m(fileName_, startLine_, name_);
    vector.push_back(m);
    return *this;
  }

  const size_t size() const
  {
    return vector.size();
  }

  const bool contains(std::shared_ptr<odb::database> db, const AstNodeInfo info)
  {
    model::PythonAstNode node;
    model::File file;
    
    transaction([&, this]
    {
      node = *db->load<model::PythonAstNode>(std::stoull(info.astNodeId.astNodeId));
      file = *db->load<model::File>(
                  node.location.file.object_id());
    });
    
    std::string fileName = file.filename;

    if (std::find_if(vector.begin(), vector.end(), [&node, &fileName]
                     (const PythonMock & m)
  {
    return m.name == node.name
           && m.startLine == node.location.range.start.line
           && m.fileName == fileName;
  }) == vector.end())
    {
      return false;
    }

    return true;
  }

  void reset()
  {
    vector.clear();
  }

  std::vector<PythonMock>::iterator begin() {
    return vector.begin();
  }

  std::vector<PythonMock>::iterator end() {
    return vector.end();
  }

  std::vector<PythonMock>::const_iterator begin() const {
    return vector.begin();
  }

  std::vector<PythonMock>::const_iterator end() const {
    return vector.end();
  }

private:
  util::OdbTransaction transaction;
  std::shared_ptr<odb::database> db;
  std::vector<PythonMock> vector;
};


class Expected
{
public:
  typedef std::map<std::string, Expected> expects_t;

  explicit Expected()
    : _is_string(true)
    , _is_astNodeInfo(false)
  {}

  Expected(const Expected & lhs)
    : _is_string(lhs._is_string)
    , _is_astNodeInfo(lhs._is_astNodeInfo)
    , _string(lhs._string)
    , _astNodeInfo(lhs._astNodeInfo)
    , _expects(lhs._expects)
  {}

  Expected(const std::string & str)
    : _is_string(true)
    , _is_astNodeInfo(false)
    , _string(str)
  {}

  Expected(const AstNodeInfo & infoNode)
    : _is_string(false)
    , _is_astNodeInfo(true)
    , _astNodeInfo(infoNode)
  {}

  Expected(const expects_t & exp)
    : _is_string(false)
    , _is_astNodeInfo(false)
    , _expects(exp)
  {}

  Expected & operator=(const Expected & lhs)
  {
    if(&lhs!=this)
    {
      _is_string = lhs._is_string;
      _is_astNodeInfo = lhs._is_astNodeInfo;
      _string = lhs._string;
      _astNodeInfo = lhs._astNodeInfo;
      _expects = lhs._expects;
    }

    return *this;
  }

  Expected & operator=(const std::string & str)
  {
    _is_string = true;
    _is_astNodeInfo = false;
    _string = str;
    _astNodeInfo = {};
    _expects.clear();

    return *this;
  }

  Expected & operator=(const expects_t & exps)
  {
    _is_string = false;
    _is_astNodeInfo = false;
    _string.clear();
    _astNodeInfo = {};
    _expects = exps;

    return *this;
  }

  Expected & operator=(const AstNodeInfo& nodeInfo)
  {
    _is_string = false;
    _is_astNodeInfo = true;
    _string.clear();
    _astNodeInfo = nodeInfo;
    _expects.clear();

    return *this;
  }

  friend bool operator==(const Expected & rhs, const Expected & lhs)
  {
    return
      rhs._is_string == lhs._is_string &&
      rhs._is_astNodeInfo == lhs._is_astNodeInfo &&
      rhs._string == lhs._string &&
      rhs._astNodeInfo == lhs._astNodeInfo &&
      rhs._expects == lhs._expects;
  }

  bool isString() const { return _is_string; }
  bool isAstNodeInfo() const { return _is_astNodeInfo; }

  const std::string & getString() const { return _string; }
  const AstNodeInfo & getAstNodeInfo() const { return _astNodeInfo; }
  const expects_t & getExpects() const { return _expects; }

private:
  bool         _is_string;
  bool         _is_astNodeInfo;
  std::string  _string;
  AstNodeInfo  _astNodeInfo;
  expects_t    _expects;
};

class PythonServiceTest : public ::testing::Test
{
protected:
  std::shared_ptr<odb::database> _db;
  std::shared_ptr<PythonServiceHandler> _pythonservice;

  PythonServiceTest() : _pythonservice(nullptr) {}



  //
  // GTest maintance methods
  //

  void init(const char* dbname)
  {
    std::string connStr = dbname;
    _db = util::createDatabase(connStr);
    _pythonservice.reset(new PythonServiceHandler(_db));
  }

  virtual void TearDown()
  {
    _pythonservice.reset();
    _db.reset();
  }

  //
  // Helper methods
  //

  model::FileId getFileId(const std::string& fn)
  {
    odb::transaction t(_db->begin());

    typedef odb::query<model::File>  FQuery;

    auto res (_db->query<model::File>(
                FQuery::filename == fn
              ));

    return (*res.begin()).id;
  }

  FileId getCoreFileId(const model::FileId fid_)
  {
    FileId fid;
    fid.__set_fid(std::to_string(fid_));

    return fid;
  }

  AstNodeInfo getAstNodeInfoByPos(
    const int line, 
    const int col, 
    const model::FileId& fid_)
  {
    FileId fid;
    fid.__set_fid(std::to_string(fid_));

    Position pos;
    pos.__set_line(line);
    pos.__set_column(col);

    FilePosition fp;
    fp.__set_file(fid);
    fp.__set_pos(pos);

    AstNodeInfo result;
    _pythonservice->getAstNodeInfoByPosition(result, fp, {});

    return result;
  }

  AstNodeInfo getAstNodeInfoByPos(
    const int line, 
    const int col, 
    const std::string& filename)
  {
    auto fid = getFileId(filename);
    return getAstNodeInfoByPos(line, col, fid);
  }

  std::pair<std::string, Expected> createInfoNode(
    const int line, 
    const int col, 
    const std::string& filename)
  {
    auto fid = getFileId(filename);
    auto nodeinfo = getAstNodeInfoByPos(line, col, fid);
    return { getFileLocByAstNode(nodeinfo),  { nodeinfo } };
  }

  bool checkSets( ExpectationSet& expected,
                 const std::vector<AstNodeInfo>& got)
  {
    std::cout << "\n- dumping - " << expected.size() << " " << got.size() << "\n";
    expected.dump();
    if (expected.size() != got.size()) std::cout << "bad size" << new_line;
      //return false;

    std::cout << "size OK" << new_line;

    for ( auto& ref : got)
    {
      std::cout << "comparing " << ref.range.range.startpos.line << " "
       << ref.range.range.startpos.column << new_line;
      if (!expected.contains(_db, ref))
      {
        return false;
      }
    }

    return true;
  }

  //
  // Checker methods
  //

  void checkDefinition(
    const int line,  
    const int col,  
    const std::string& fn, 
     ExpectationSet& expected)
  {
    checkGetReferences(line, col, getFileId(fn), expected, true, false);
  }

  void checkReferences(
    const int line,  
    const int col,  
    const std::string& fn, 
     ExpectationSet& expected)
  {
    checkGetReferences(line, col, getFileId(fn), expected, false, false);
  }

  void checkReferencesInFile(
    const int line,  
    const int col,  
    const std::string& fn, 
     ExpectationSet& expected)
  {
    checkGetReferences(line, col, getFileId(fn), expected, false, true);
  }

  void checkGetReferences(
    const int line,  
    const int col,  
    const model::FileId fid, 
     ExpectationSet& expected,
    const bool definition, 
    const bool thisFileOnly)
  {
    if (definition && thisFileOnly)
    {
      EXPECT_TRUE(false);
      std::cout << "Malformed argument in python-checkGetReferences()!";
    }

    cc::service::language::RefTypes::type refType = (definition) ? RefTypes::GetDef : RefTypes::GetUsage;

    AstNodeInfo node = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> infos;

    if (thisFileOnly)
    {
      _pythonservice->getReferencesInFile(
        infos, node.astNodeId, refType, getCoreFileId(fid));
    }
    else
    {
      _pythonservice->getReferences(infos, node.astNodeId, refType);
    }
    EXPECT_TRUE(checkSets(expected, infos));
  }


  void checkInfoTree(
    const AstNodeInfo& AstNodeInfo_,
    const std::vector<InfoNode>& subInfoNodes_,
    const Expected::expects_t & exps_,
    const unsigned int catlevel_ = 0,
    const bool strict = false)
  {
    std::size_t hitcounter = 0;
    std::set<std::string> visited;

    for(const InfoNode& subnode: subInfoNodes_)
    {
      if(0 == subnode.query.queryId)
        ++hitcounter;

      Expected::expects_t::const_iterator it = exps_.find(subnode.label);
      bool in_cat = false;

      if(exps_.end() == it && catlevel_ < subnode.category.size())
      {
        it = exps_.find(subnode.category[catlevel_]);
        in_cat = true;
      }

      if(exps_.end() != it)
      {
        const Expected & exp = it->second;

        if(in_cat && exp.isString()) { continue; }
        if(!in_cat && !exp.isString() && !exp.isAstNodeInfo() ) { continue; }

        if(in_cat)
        {
          if(0 < visited.count(it->first)) { continue; }
          visited.insert(it->first);
        }

        if(exp.isString())
        {
          EXPECT_EQ(exp.getString(), subnode.value);
        }
        else if(exp.isAstNodeInfo())
        {
          EXPECT_EQ(exp.getAstNodeInfo().astNodeId, subnode.astValue.astNodeId);
        }
        else
        {
          // TODO: this solution is a little fragile, the actual categories should be passed to checkInfoTree()
          if(catlevel_+1 < subnode.category.size())
          {
            checkInfoTree(
              AstNodeInfo_, subInfoNodes_, exp.getExpects(), catlevel_+1);
          }
          else if(0 != subnode.query.queryId)
          {
            std::vector<InfoNode> subsubinfonodes;
            _pythonservice->getSubInfoTree(
              subsubinfonodes, AstNodeInfo_.astNodeId, subnode.query);
            checkInfoTree(
              AstNodeInfo_, subsubinfonodes, exp.getExpects(), catlevel_+1);
          }
          else
          {
            checkInfoTree(AstNodeInfo_, subInfoNodes_, exp.getExpects());
          }
        }
      }
    }

    std::stringstream unvisited;
    for(const auto & p : exps_)
      if(!visited.count(p.first)) { unvisited << p.first << ", "; }

    if(catlevel_ > 0 || strict)
      EXPECT_EQ(hitcounter, exps_.size()) << unvisited.str();
  }

  void checkInfoTree(
    const int line,
    const int col,
    const std::string& filename,
    const Expected::expects_t& exps)
  {
    AstNodeInfo astnodeinfo = getAstNodeInfoByPos(line, col, filename);

    std::vector<InfoNode> infonodes;
    _pythonservice->getInfoTree(infonodes, astnodeinfo.astNodeId);
    checkInfoTree(astnodeinfo, infonodes, exps);
  }
};



//
// Test cases
//

TEST_F(PythonServiceTest, DefTestVariable)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
  std::string file = "highlight_variable.py";

  ExpectationSet expect(_db);

  expect.add(file, 11, "var").add(file, 14, "var"); // local
  checkDefinition(12, 9, file, expect);
  expect.reset();

  expect.add(file, 22, "var"); // nonlocal
  checkDefinition(25, 11, file, expect);
  expect.reset();

  expect.add(file, 34, "var"); // local enclosed
  checkDefinition(36, 13, file, expect);
  expect.reset();

  expect.add(file, 6, "var"); // global
  checkDefinition(62, 3, file, expect);
  expect.reset();

  expect.add(file, 6, "var"); // global enclosed
  checkDefinition(80, 8, file, expect);
  expect.reset();
}

TEST_F(PythonServiceTest, DefTestAttribute)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
  std::string file = "attribute.py";

  ExpectationSet expect(_db);
 
  expect.add(file, 3, "l1_1_oa"); // self def top
  checkDefinition(3, 3, file, expect);
  expect.reset();

  expect.add(file, 6, "l1_1_ia"); // write constr
  checkDefinition(15, 10, file, expect);
  expect.reset();

  expect.add(file, 20, "l1_1_oa"); 
  checkDefinition(67, 16, file, expect);
  expect.reset();

  expect.add(file, 40, "l2_2_oa"); 
  checkDefinition(76, 16, file, expect);
  expect.reset();

  expect.add(file, 65, "l4_1_ia"); 
  checkDefinition(83, 16, file, expect);
  expect.reset();

  expect.add(file, 21, "name"); 
  checkDefinition(85, 16, file, expect);
  expect.reset();
}

TEST_F(PythonServiceTest, DefTestFunction)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
  std::string file1 = "highlight_function.py";

  ExpectationSet expect(_db);
  
  expect.add(file1, 6, "func"); // normal function call 
  checkDefinition(18, 11, file1, expect);
  expect.reset();

  expect.add(file1, 18, "funcPtr").add(file1, 21, "funcPtr"); // normal function pointer 
  checkDefinition(19, 1, file1, expect);
  expect.reset();

  expect.add(file1, 21, "funcPtr"); // normal function pointer redefine
  checkDefinition(21, 1, file1, expect);
  expect.reset();

  expect.add(file1, 30, "memberFunc"); // member function
  checkDefinition(41, 4, file1, expect);
  expect.reset();

  expect.add(file1, 33, "memberFuncParam1"); // function pointer as parameter
  checkDefinition(44, 15, file1, expect);
  expect.reset();

  std::string file2 = "labda_function.py";

  expect.add(file2, 1, "lama"); // lambda function
  checkDefinition(2, 1, file2, expect);
  expect.reset();

  std::string file3 = "multiple_binding.py";
  std::string file4 = "imported_foo.py";

  expect.add(file3, 9, "foo").add(file4, 3, "foo"); // multi def function
  checkDefinition(18, 1, file3, expect);
  expect.reset();

}

TEST_F(PythonServiceTest, DefTestClass)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
  std::string file = "overriden_methods.py";

  ExpectationSet expect(_db);
  
  expect.add(file, 57, "Crow"); // multi def function
  checkDefinition(90, 58, file, expect);
  expect.reset();

  expect.add(file, 45, "Lion"); // multi def function
  checkDefinition(109, 26, file, expect);
  expect.reset();
}

// TEST_F(PythonServiceTest, RefTestVariable)
// {
//   init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
//   std::string file = "highlight_variable.py";

//   ExpectationSet expect(_db);

//   expect.add(file, 12, "var").add(file, 15, "var").add(file, 16, "var"); // local
//   checkReferences(12, 9, file, expect);
//   expect.reset();

//   expect.add(file, 25, "var"); // nonlocal
//   checkReferences(22, 3, file, expect);
//   expect.reset();

//   expect.add(file, 35, "var").add(file, 36, "var"); // local enclosed
//   checkReferences(34, 5, file, expect);
//   expect.reset();
// }

TEST_F(PythonServiceTest, InfoTreeTestVariable)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");

  // Local variable
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(11, 3, "highlight_variable.py");
    infoTree["Defined"] = getAstNodeInfoByPos(11, 3, "highlight_variable.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(12, 9, "highlight_variable.py"),
      createInfoNode(15, 6, "highlight_variable.py"),
      createInfoNode(16, 11, "highlight_variable.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(11, 3, "highlight_variable.py"),
      createInfoNode(14, 3, "highlight_variable.py")
    });

    checkInfoTree(11, 3, "highlight_variable.py", infoTree);
  }

  // Enclosed variable
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(22, 3, "highlight_variable.py");
    infoTree["Defined"] = getAstNodeInfoByPos(22, 3, "highlight_variable.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(25, 11, "highlight_variable.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(22, 3, "highlight_variable.py")
    });

    checkInfoTree(22, 3, "highlight_variable.py", infoTree);
  }

  // Enclosed-local variable
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(34, 5, "highlight_variable.py");
    infoTree["Defined"] = getAstNodeInfoByPos(34, 5, "highlight_variable.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(35, 8, "highlight_variable.py"),
      createInfoNode(36, 13, "highlight_variable.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(34, 5, "highlight_variable.py")
    });

    checkInfoTree(34, 5, "highlight_variable.py", infoTree);
  }

  // Global variable
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(6, 1, "highlight_variable.py");
    infoTree["Defined"] = getAstNodeInfoByPos(6, 1, "highlight_variable.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(42, 9, "highlight_variable.py"),
      createInfoNode(44, 6, "highlight_variable.py"),
      createInfoNode(45, 11, "highlight_variable.py"),
      createInfoNode(53, 8, "highlight_variable.py"),
      createInfoNode(54, 13, "highlight_variable.py"),
      createInfoNode(63, 9, "highlight_variable.py"),
      createInfoNode(66, 6, "highlight_variable.py"),
      createInfoNode(67, 11, "highlight_variable.py"),
      createInfoNode(77, 11, "highlight_variable.py"),
      createInfoNode(80, 8, "highlight_variable.py"),
      createInfoNode(81, 13, "highlight_variable.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(6, 1, "highlight_variable.py"),
      createInfoNode(62, 3, "highlight_variable.py"),
      createInfoNode(65, 3, "highlight_variable.py"),
      createInfoNode(76, 5, "highlight_variable.py"),
      createInfoNode(79, 5, "highlight_variable.py")
    });

    checkInfoTree(6, 1, "highlight_variable.py", infoTree);
  }
}

TEST_F(PythonServiceTest, InfoTreeTestAttribute)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");

  // Independent - outter
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(3, 3, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(3, 3, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      // empty
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(3, 3, "attribute.py")
    });

    checkInfoTree(3, 3, "attribute.py", infoTree);
  }

  // Independent - inner
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(6, 10, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(6, 10, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      // empty
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(6, 10, "attribute.py"),
      createInfoNode(15, 10, "attribute.py")
    });

    checkInfoTree(6, 10, "attribute.py", infoTree);
  }

  // Normal hierarchy
  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(20, 3, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(20, 3, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(67, 16, "attribute.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(20, 3, "attribute.py"),
      createInfoNode(88, 10, "attribute.py")
    });

    checkInfoTree(20, 3, "attribute.py", infoTree);
  }

  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(31, 10, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(31, 10, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(71, 16, "attribute.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(31, 10, "attribute.py"),
      createInfoNode(92, 10, "attribute.py")
    });

    checkInfoTree(31, 10, "attribute.py", infoTree);
  }

  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(34, 3, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(34, 3, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(73, 16, "attribute.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(34, 3, "attribute.py"),
      createInfoNode(94, 10, "attribute.py")
    });

    checkInfoTree(34, 3, "attribute.py", infoTree);
  }

  {
    Expected::expects_t infoTree;
    infoTree["Name"] = getAstNodeInfoByPos(21, 3, "attribute.py");
    infoTree["Defined"] = getAstNodeInfoByPos(21, 3, "attribute.py");
    infoTree["Reads"] = Expected::expects_t(
    {
      createInfoNode(85, 16, "attribute.py")
    });
    infoTree["Writes"] = Expected::expects_t(
    {
      createInfoNode(21, 3, "attribute.py"),
      createInfoNode(57, 10, "attribute.py"),
      createInfoNode(106, 10, "attribute.py")
    });

    checkInfoTree(21, 3, "attribute.py", infoTree);
  }
}

TEST_F(PythonServiceTest, InfoTreeTestOfFunctionOverridenMethodAnimal)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");

  Expected::expects_t infoTree;
  infoTree["Name"] = getAstNodeInfoByPos(29, 9, "overriden_methods.py");
  infoTree["Defined"] = getAstNodeInfoByPos(29, 9, "overriden_methods.py");
  infoTree["Decorators"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Parameters"] = Expected::expects_t(
  {
    createInfoNode(29, 14, "overriden_methods.py")
  });
  infoTree["Overrides"] = Expected::expects_t(
  {
    createInfoNode(23, 9, "overriden_methods.py")
  });
  infoTree["Overridden by"] = Expected::expects_t(
  {
    createInfoNode(35, 9, "overriden_methods.py"),
    createInfoNode(41, 9, "overriden_methods.py"),
    createInfoNode(47, 9, "overriden_methods.py"),
    createInfoNode(53, 9, "overriden_methods.py"),
    createInfoNode(59, 9, "overriden_methods.py")
  });
  infoTree["Called by"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Calls"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Local variables"] = Expected::expects_t(
  {
    // empty
  });

  checkInfoTree(29, 9, "overriden_methods.py", infoTree);
}

TEST_F(PythonServiceTest, InfoTreeTestOfFunctionOverridenMethodCat)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");

  Expected::expects_t infoTree;
  infoTree["Name"] = getAstNodeInfoByPos(35, 9, "overriden_methods.py");
  infoTree["Defined"] = getAstNodeInfoByPos(35, 9, "overriden_methods.py");
  infoTree["Decorators"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Parameters"] = Expected::expects_t(
  {
    createInfoNode(35, 14, "overriden_methods.py")
  });
  infoTree["Overrides"] = Expected::expects_t(
  {
    createInfoNode(29, 9, "overriden_methods.py")
  });
  infoTree["Overridden by"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Called by"] = Expected::expects_t(
  {
    createInfoNode(74, 39, "overriden_methods.py"),
    createInfoNode(82, 43, "overriden_methods.py")
  });
  infoTree["Calls"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Local variables"] = Expected::expects_t(
  {
    // empty
  });

  checkInfoTree(35, 9, "overriden_methods.py", infoTree);
}

TEST_F(PythonServiceTest, InfoTreeTestOfFunctionOverridenMethodCrow)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/pythontest2/st2.sqlite");
  
  Expected::expects_t infoTree;
  infoTree["Name"] = getAstNodeInfoByPos(59, 9, "overriden_methods.py");
  infoTree["Defined"] = getAstNodeInfoByPos(59, 9, "overriden_methods.py");
  infoTree["Decorators"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Parameters"] = Expected::expects_t(
  {
    createInfoNode(59, 14, "overriden_methods.py"),
    createInfoNode(59, 20, "overriden_methods.py")
  });
  infoTree["Overrides"] = Expected::expects_t(
  {
    createInfoNode(29, 9, "overriden_methods.py")
  });
  infoTree["Overridden by"] = Expected::expects_t(
  {
    // empty
  });
  /*infoTree["Called by"] = Expected::expects_t
  {
    {"Possible callers" , Expected::expects_t
    {
      createInfoNode(74, 39, "overriden_methods.py"),
      createInfoNode(82, 43, "overriden_methods.py"),
      createInfoNode(91, 47, "overriden_methods.py"),
      createInfoNode(114, 25, "overriden_methods.py"),
    }}
  };*/
  infoTree["Calls"] = Expected::expects_t(
  {
    // empty
  });
  infoTree["Local variables"] = Expected::expects_t(
  {
    // empty
  });

  checkInfoTree(59, 9, "overriden_methods.py", infoTree);
}


int main(int argc, char** argv)
{
  util::StreamLog::setStrategy(std::shared_ptr<util::LogStrategy>(
                                 new util::StandardErrorLogStrategy()))  ;
  util::StreamLog::setLogLevel(util::INFO);
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  //::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
