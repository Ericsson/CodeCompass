#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_USE_OWN_TR1_TUPLE 0

#include <set>
#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <model/file.h>

#include "../../cppservice/src/cppservice.h"
#include "language-api/LanguageService.h"

#include <util/util.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

using namespace cc::service::language;
using namespace cc::service::core;
using namespace cc;

class CppServiceTest : public ::testing::Test
{
protected:
  std::shared_ptr<odb::database> _db;
  std::shared_ptr<CppServiceHelper> _cppserviceHelper;
  std::shared_ptr<CppServiceHandler> _cppservice;

  CppServiceTest() : _cppserviceHelper(nullptr), _cppservice(nullptr) {}

  virtual void TearDown()
  {
    _cppservice.reset();
    _cppserviceHelper.reset();
    _db.reset();
  }

  void init(const char* dbname)
  {
    std::string connStr = dbname;
    _db = util::createDatabase(connStr);
    _cppserviceHelper.reset( new CppServiceHelper(_db) );
    _cppservice.reset(new CppServiceHandler(*_cppserviceHelper));
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

  void checkDefinition(int line, int col, model::FileId fid, std::vector<int> expected_lines, int exptected_value = 1)
  {
    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> results;
    _cppservice->getReferences(results, anFrom.astNodeId, RefTypes::GetDef);
    std::sort(results.begin(), results.end(), [](const auto& lhs, const auto& rhs){
      return lhs.range.range.startpos.line < rhs.range.range.startpos.line;
    });
    //AstNodeInfo anTo = _cppservice->getDefinition(anFrom.astNodeId);
     EXPECT_EQ(exptected_value, results.size());
    for(size_t i = 0; i < results.size(); ++i)
      EXPECT_EQ(expected_lines[i], results[i].range.range.startpos.line);
  }

  bool checkReferences(int line, int col, model::FileId fid, const std::set<int>& expected)
  {
    FileId fileId;
    fileId.__set_fid(std::to_string(fid));

    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid);
    std::vector<AstNodeInfo> refs;
    _cppservice->getReferencesInFile(refs, anFrom.astNodeId, RefTypes::GetUsage, fileId);

    return checkSets(expected, refs);
  }
  
  bool checkCallerFunctions(int line, int col, model::FileId fid, const std::set<int>& expected)
  {
    FileId fileId;
    fileId.__set_fid(std::to_string(fid));

    AstNodeInfo anFrom = getAstNodeInfoByPos(line, col, fid); // def of setPrivX
    std::vector<AstNodeInfo> refs = _cppserviceHelper->getCallerFunctions(anFrom.astNodeId, fileId);

    return checkSets(expected, refs);
  }

  bool checkSets(const std::set<int>& expected, const std::vector<AstNodeInfo>& got)
  {
    if (expected.size() != got.size())
      return false;

    for (const auto& ref : got)
    {
      if (expected.count(ref.range.range.startpos.line) == 0)
        return false;
    }

    return true;
  }

  bool checkClass(
    std::string testDir,
    int line,
    int col,
    std::string qname,
    std::string defined,
    int inheritsFromNum,
    int inheritedByNum,
    int friendsNum,
    std::set<std::string> publicMethodLabels,
    std::set<std::tuple<std::string, std::string, std::string> > members // visibilityMod, retVal, name
  )
  {
    std::string s1 = "sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/";
    std::string s2 = s1 + testDir + "/" + testDir + ".sqlite";
    init(s2.c_str());
    auto fileId = getFileId(testDir + ".cpp");
    AstNodeInfo localClass = getAstNodeInfoByPos(line, col, fileId);

    std::vector<InfoNode> infonodes;
    _cppservice->getInfoTree(infonodes, localClass.astNodeId);

    int hitcounter = 0;

    for (const InfoNode& node : infonodes)
    {
      if (node.label == "Qualified Name")
      {
        if (!(qname == node.value)) // TODO: Why not operator!= ?
        {
          std::cout << "Failed at Qualified Name!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Defined")
      {
        if (!(defined == node.value))
        {
          std::cout << "Failed at Defined!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Inherits From")
      {
        std::vector<InfoNode> inherits;
        _cppservice->getSubInfoTree(inherits, localClass.astNodeId, node.query);
        if (!(inheritsFromNum == inherits.size()))
        {
          std::cout << "Failed at Inherits From!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Inherited By")
      {
        std::vector<InfoNode> inherited;
        _cppservice->getSubInfoTree(inherited, localClass.astNodeId, node.query);
        if (!( inheritedByNum == inherited.size() ))
        {
          std::cout << "Failed at Inherited By!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Friends")
      {
        std::vector<InfoNode> friends;
        _cppservice->getSubInfoTree(friends, localClass.astNodeId, node.query);
        if (!( friendsNum == friends.size() ))
        {
          std::cout << "Failed at Friends!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Methods")
      {
        std::vector<InfoNode> subsets;
        _cppservice->getSubInfoTree(subsets, localClass.astNodeId, node.query);
        for (const InfoNode& node : subsets)
        {
          if(0>node.astValue.range.range.startpos.line)
          {
            // the implicit functions don't have definition
            continue;
          }

          if (node.category.front() == "public" && node.label != "")
          {
            if (!( publicMethodLabels.count(node.label) > 0 ))
            {
              std::cout << "Failed at MethodsPublic!\n";
              return false;
            }
          }
          else if (node.category.front() == "Static" && node.label != "")
          {
            if (!( publicMethodLabels.count(node.label) > 0 ))
            {
              std::cout << "Failed at MethodsStatic!\n";
              return false;
            }
          }
          else if (node.category.front() == "Inherited" && node.label != "")
          {
            if (!( publicMethodLabels.count(node.label) > 0 ))
            {
              std::cout << "Failed at MethodsInherited!\n";
              return false;
            }
          }
        }
        ++hitcounter;
      }


      else if (!node.category.empty() && node.category.front() == "Members")
      {
        std::vector<InfoNode> subsets;
        _cppservice->getSubInfoTree(subsets, localClass.astNodeId, node.query);

        for (auto iNode : subsets)
        {
          bool found = false;
          std::string vis = iNode.category.front();
          std::string val = iNode.value;
          std::string nam = iNode.label;

          if (vis == "Inherited")
          {
            std::vector<InfoNode> inheritedNodes;
            _cppservice->getSubInfoTree(inheritedNodes, localClass.astNodeId, iNode.query);

            for (auto inherited : inheritedNodes)
            {

              bool found = false;
              std::string vis = inherited.category.front();
              std::string val = inherited.value;
              std::string nam = inherited.label;

              for (auto tup : members)
              {
                std::string expVis = std::get<0>(tup);
                std::string expVal = std::get<1>(tup);
                std::string expNam = std::get<2>(tup);

                if (
                  (vis == expVis ) &&
                  (val == expVal) &&
                  (nam == expNam)
                )
                {
                  found = true;
                }
              }

              if (found == false)
              {
                std::cout << "Failed at Members Inherited!\n";
                return false;
              }
            }
          } else {
            for (auto tup : members)
            {
              std::string expVis = std::get<0>(tup);
              std::string expVal = std::get<1>(tup);
              std::string expNam = std::get<2>(tup);

              if (
                (vis == expVis ) &&
                (val == expVal) &&
                (nam == expNam)
              )
              {
                found = true;
              }
            }

            if (found == false)
            {
              std::cout << "Failed at Members!\n";
              return false;
            }
          }
        }
        ++hitcounter;
      }
    }
    if (!( 7 == hitcounter ))
    {
      std::cout << "Failed at hitcounter!\n";
      return false;
    }
    else
    {
      return true;
    }
  }

  bool checkVariable(
    std::string testDir,
    int line,
    int col,
    std::string name,
    std::string qname,
    std::string type,
    std::string declaration,
    int callsNum,
    int readsNum,
    std::string writesInFile,
    std::set<std::string> writeNames,
    std::set<std::string> writeLabels
  )
  {
    std::string s1 = "sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/";
    std::string s2 = s1 + testDir + "/" + testDir + ".sqlite";
    init(s2.c_str());
    auto fileId = getFileId(testDir + ".cpp");
    AstNodeInfo localVar2 = getAstNodeInfoByPos(line, col, fileId);

    std::vector<InfoNode> infonodes;
    _cppservice->getInfoTree(infonodes, localVar2.astNodeId);

    int hitcounter = 0;
    for (const InfoNode& node : infonodes)
    {
      if (node.label == "Name")
      {
        if (!(name == node.value)) // TODO: Why not operator!= ?
        {
          std::cout << "Failed at Name!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Qualified Name")
      {
        if (!(qname == node.value))
        {
          std::cout << "Failed at Qualified Name!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Type")
      {
        if (!(type == node.value))
        {
          std::cout << "XXX: \"" + type + "\" vs \"" + node.value + "\"";
          std::cout << "Types: \"" + type + "\" vs \"" + node.astValue.astNodeValue + "\"";
          std::cout << "Failed at Type!\n" + node.astValue.astNodeValue + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Declaration")
      {
        if (!(declaration == node.value))
        {
          std::cout << "Failed at Declaration!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Calls")
      {
        std::vector<InfoNode> calls;
        _cppservice->getSubInfoTree(calls, localVar2.astNodeId, node.query);
        if (!(callsNum == calls.size()))
        {
          std::cout << "Failed at Calls!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Reads")
      {
        std::vector<InfoNode> reads;
        _cppservice->getSubInfoTree(reads, localVar2.astNodeId, node.query);
        if (!(readsNum == reads.size()))
        {
          std::cout << "Failed at Reads!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Writes")
      {
        std::vector<InfoNode> writes_bound;
        _cppservice->getSubInfoTree(writes_bound, localVar2.astNodeId, node.query);


        if (writes_bound.size() > 0)
        {
          if (!(writesInFile == writes_bound.front().category.front()))
          {
            std::cout << "Failed at WritesFile!\n";
            return false;
          }


          std::vector<InfoNode> writes;
          _cppservice->getSubInfoTree(writes, localVar2.astNodeId, writes_bound.front().query);
          for (const InfoNode& node : writes)
          {
            if (!(writeNames.count(node.value) > 0) ||
                !(writeLabels.count(node.label) > 0))
            {
              std::cout << "Failed at Writes!\n";
              std::cout << "Types: \"" + node.value + "\" vs \"" + node.label + "\" ";
              return false;
            }
          }
          if (!(writeNames.size() == writes.size()))
          {
            std::cout << "Failed at WritesSize!\n";
            return false;
          }
        }
        else
        {
          if (writesInFile != "")
          {
            return false;
          }
        }
        ++hitcounter;
      } // END WRITES
    }
    if (!(6 == hitcounter))
    {
      std::cout << "Failed at hitcounter!\n";
      return false;
    }
    return true; // all is fine
  }

  bool checkFunction(
    std::string testDir,
    int line,
    int col,
    std::string name,
    std::string qname,
    std::string signature,
    std::string returnType,
    std::string defined,
    int declNum,
    std::set<std::string> paramNames,
    std::set<std::string> localVarNames,
    int calleesNum,
    int callersNum,
    int assignedNum
  )
  {
    std::string s1 = "sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/";
    std::string s2 = s1 + testDir + "/" + testDir + ".sqlite";
    init(s2.c_str());
    auto fileId = getFileId(testDir + ".cpp");
    AstNodeInfo func = getAstNodeInfoByPos(line, col, fileId);

    std::vector<InfoNode> infoNodes;
    _cppservice->getInfoTree(infoNodes, func.astNodeId);

    int hitcounter = 0;
    for (const InfoNode& node : infoNodes)
    {
      if (node.label == "Name")
      {
        if (!(name == node.value)) // TODO: Why not operator!= ?
        {
          std::cout << "Failed at Name!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Qualified name")
      {
        if (!(qname == node.value))
        {
          std::cout << "Failed at Qualified Name!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Signature")
      {
        if (!(signature == node.value))
        {
          std::cout << "Failed at Signature!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Return Type")
      {
        if (!(returnType == node.value))
        {
          std::cout << "Failed at Type!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (node.label == "Defined")
      {
        if (!(defined == node.value))
        {
          std::cout << "Failed at Defined!\n" + node.value + "|";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Declarations")
      {
        std::vector<InfoNode> decls;
        _cppservice->getSubInfoTree(decls, func.astNodeId, node.query);
        if (!(declNum == decls.size()))
        {
          std::cout << "Failed at Declarations!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Parameters")
      {

        std::vector<InfoNode> params;
        _cppservice->getSubInfoTree(params, func.astNodeId, node.query);

        if (paramNames.size() != params.size())
        {
          std::cout << "Failed at ParametersSize!\n";
          return false;
        }

        for (const InfoNode& node : params)
        {
          if (!(paramNames.count(node.value) > 0))
          {
            std::cout << "Failed at Parameters!\n";
            return false;
          }
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Local Variables")
      {

        std::vector<InfoNode> locals;
        _cppservice->getSubInfoTree(locals, func.astNodeId, node.query);

        if (localVarNames.size() != locals.size())
        {
          std::cout << "Failed at Local VariablesSize!\n" << localVarNames.size() << locals.size();
          return false;
        }

        for (const InfoNode& node : locals)
        {
          if (!(localVarNames.count(node.value) > 0))
          {
            std::cout << "Failed at Local Variables!\n";
            return false;
          }
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Callees")
      {

        std::vector<InfoNode> callees;
        _cppservice->getSubInfoTree(callees, func.astNodeId, node.query);

        if (calleesNum != countUsageLinesInFolders(callees, func.astNodeId))
        {
          std::cout << "Failed at Callees!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Callers")
      {

        std::vector<InfoNode> callers;
        _cppservice->getSubInfoTree(callers, func.astNodeId, node.query);

        if (callersNum != countUsageLinesInFolders(callers, func.astNodeId))
        {
          std::cout << callersNum << countUsageLinesInFolders(callers, func.astNodeId);
          std::cout << "Failed at Callers!\n";
          return false;
        }
        ++hitcounter;
      }
      else if (!node.category.empty() && node.category.front() == "Assigned To Function Pointer")
      {

        std::vector<InfoNode> assigned;
        _cppservice->getSubInfoTree(assigned, func.astNodeId, node.query);

        if (assignedNum != countUsageLinesInFolders(assigned, func.astNodeId))
        {
          std::cout << assignedNum << countUsageLinesInFolders(assigned, func.astNodeId);
          std::cout << "Failed at Assigned To Function Pointer!\n";
          return false;
        }
        ++hitcounter;
      }
    }
    if (!(11 == hitcounter))
    {
      std::cout << "Failed at hitcounter!\n" << hitcounter;
      return false;
    }
    return true; // all is fine
  }

  //func.astNodeId
  int countUsageLinesInFolders(std::vector<InfoNode> iNodes, ::cc::service::core::AstNodeId nodeId)
  {
    int cnt = 0;
    for (const InfoNode& node : iNodes)
    {
      for (std::string s : node.category)
      {
        if (hasEnding(s, ".cpp") ||
            hasEnding(s, ".cxx") ||
            hasEnding(s, ".tcc") ||
            hasEnding(s, ".cc") ||
            hasEnding(s, ".c") ||
            hasEnding(s, ".h") ||
            hasEnding(s, ".hpp") ||
            hasEnding(s, ".hxx") ||
            hasEnding(s, ".hh"))
        {
          std::vector<InfoNode> vec;
          _cppservice->getSubInfoTree(vec, nodeId, node.query);

          for (const InfoNode& n : vec)
          {
            if (n.label != "") {
              cnt++;
            }
          }
        }
        else
        {
          cnt++;
        }
      }
    }
    return cnt;
  }

  bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString == "")
    {
      return false;
    }

    std::vector<std::string> strs = split(fullString, ' ');

    if (strs[0].length() >= ending.length()) {
      return (0 == strs[0].compare (strs[0].length() - ending.length(), ending.length(), ending));
    } else {
      return false;
    }
  }

  std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }


  std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
  }

  bool checkInfoBox(
    std::string testDir,
    int line,
    int col,
    std::string expectedLine)
  {
    std::string s1 = "sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/";
    std::string s2 = s1 + testDir + "/" + testDir + ".sqlite";
    init(s2.c_str());
    auto fileId = getFileId(testDir + ".cpp");
    AstNodeInfo nodeInfo = getAstNodeInfoByPos(line, col, fileId);

    InfoBox infoBox = _cppserviceHelper->getInfoBox(nodeInfo.astNodeId);

    if (infoBox.information == "")
    {
      return false;
    }

    std::string first_line(infoBox.information.begin(), std::find(infoBox.information.begin(), infoBox.information.end(), '\n'));

    if (!(first_line == expectedLine))
    {
      return false;
    }

    return true;
  }
};

TEST_F(CppServiceTest, getDefinitionSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fileId = getFileId("simple.cpp");

  checkDefinition(24, 13, fileId, {33}); // _privX
  checkDefinition(43, 13, fileId, {27}); // localVar2.setPrivX(0); call
  checkDefinition(42, 4, fileId, {4}); // LocalClass localVar2; type
}

TEST_F(CppServiceTest, getDefinitionTinyXml)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");
  auto tinyxml2 = getFileId("tinyxml2.cpp");
  auto xmltest = getFileId("xmltest.cpp");

  checkDefinition(98, 7, tinyxml2, {178}); // _end
  checkDefinition(125, 27, tinyxml2, {119}); // length
  checkDefinition(117, 41, xmltest, {636}); // FirstChild
  checkDefinition(257, 41, xmltest, {677}); // InsertFirstChild
  checkDefinition(346, 9, xmltest, {1357}); // LocalClass localVar2; type
}

TEST_F(CppServiceTest, getAstNodeInfoByPosition)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fileId = getFileId("simple.cpp");

  {
    AstNodeInfo ani = getAstNodeInfoByPos(24, 13, fileId);
    EXPECT_EQ("Variable", ani.astNodeType);
    EXPECT_EQ("(anonymous namespace)::LocalClass::_privX", ani.astNodeValue);
  }

  {
    AstNodeInfo ani = getAstNodeInfoByPos(4, 7, fileId);
    EXPECT_EQ("Type", ani.astNodeType);
    EXPECT_EQ("LocalClass", ani.astNodeValue);
  }

  {
    AstNodeInfo ani = getAstNodeInfoByPos(38, 6, fileId);
    EXPECT_EQ("Function", ani.astNodeType);
    EXPECT_EQ("main", ani.astNodeValue);
  }

  {
    AstNodeInfo ani = getAstNodeInfoByPos(38, 15, fileId);
    EXPECT_EQ("Variable", ani.astNodeType);
    EXPECT_EQ("argc_", ani.astNodeValue);
  }

  {
    AstNodeInfo ani = getAstNodeInfoByPos(38, 15, fileId);
    EXPECT_EQ("Variable", ani.astNodeType);
    EXPECT_EQ("argc_", ani.astNodeValue);
  }

  /* TODO: int x = 0; eseten az int-et kijelolve az x-et kapjuk vissza ez meg nincs jol implementalva
  {
    AstNodeInfo ani = getAstNodeInfoByPos(40, 4);
    EXPECT_EQ("Type",ani.astNodeType);
    EXPECT_EQ("int", ani.astNodeValue);
  }
  */

  /* TODO: itt a return 0-an a return-re kattintottunk, de mivel csak azokat az astNode-okat taroljuk, amelyiknek
   * van mangled neve, a legszukebb igy a main lett
  {
    AstNodeInfo ani = getAstNodeInfoByPos(46, 4);
    EXPECT_EQ("",ani.astNodeType);
    EXPECT_EQ("", ani.astNodeValue);
  }
  */

}

TEST_F(CppServiceTest, getReferencesSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fid = getFileId("simple.cpp");

  EXPECT_TRUE(checkReferences(33, 11, fid, {8, 19, 24, 29, 33})); // def of _privX
  EXPECT_TRUE(checkReferences(27, 11, fid, {27, 43})); //def of setPrivX

}

TEST_F(CppServiceTest, getReferencesTinyXml)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/tinyxml/tinyxml.sqlite");
  auto tinyxml2_h_id = getFileId("tinyxml2.h");

  EXPECT_TRUE(checkReferences(143, 12, tinyxml2_h_id, {143})); // def of Set
}

TEST_F(CppServiceTest, getReferencesPolymorphicCall)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/cpptest/cpptest.sqlite");
  auto testcpp_cpp_id = getFileId("cpptest.cpp");

  EXPECT_TRUE(checkReferences(59, 17, testcpp_cpp_id, {59, 96, 105, 153}));
}

TEST_F(CppServiceTest, getCallerFunctionsSimple)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fid = getFileId("simple.cpp");

  EXPECT_TRUE( checkCallerFunctions(27, 11, fid, {43}) ); //def od setPrivX
  EXPECT_TRUE( checkCallerFunctions(17, 11, fid, {44}) ); // def of getPrivX
}


TEST_F(CppServiceTest, InfoTreeOfVariable)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fileId = getFileId("simple.cpp");
  AstNodeInfo locaVar2 = getAstNodeInfoByPos(42, 16, fileId);

  std::vector<InfoNode> infonodes;
  _cppservice->getInfoTree(infonodes, locaVar2.astNodeId);

  int hitcounter = 0;
  for (const InfoNode& node : infonodes)
  {
    if (node.label == "Name")
    {
      EXPECT_EQ(std::string("locaVar2"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Qualified Name")
    {
      EXPECT_EQ(std::string("locaVar2"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Type")
    {
      EXPECT_EQ(std::string("LocalClass"), node.astValue.astNodeValue);
      ++hitcounter;
    }
    else if (node.label == "Declaration")
    {
      EXPECT_EQ(std::string("simple.cpp:42:3"), node.value);
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Reads")
    {
      std::vector<InfoNode> reads;
      _cppservice->getSubInfoTree(reads, locaVar2.astNodeId, node.query);
      EXPECT_EQ(0, reads.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Writes")
    {
      std::vector<InfoNode> writes_bound;
      _cppservice->getSubInfoTree(writes_bound, locaVar2.astNodeId, node.query);
      EXPECT_EQ("simple.cpp (2)", writes_bound.front().category.front());
      std::vector<InfoNode> writes;
      _cppservice->getSubInfoTree(writes, locaVar2.astNodeId, writes_bound.front().query);
      std::set<std::string> expected{"  locaVar2.getPrivX();", "  locaVar2.setPrivX(0);"};
      std::set<std::string> expLabels{"43:3", "44:3"};

      for (const InfoNode& node : writes)
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

TEST_F(CppServiceTest, InfoTreeOfTypehandler)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fileId = getFileId("simple.cpp");
  AstNodeInfo localClass = getAstNodeInfoByPos(4, 8, fileId);

  std::vector<InfoNode> infonodes;
  _cppservice->getInfoTree(infonodes, localClass.astNodeId);

  int hitcounter = 0;
  for (const InfoNode& node : infonodes)
  {
    if (node.label == "Qualified Name")
    {
      EXPECT_EQ(std::string("(anonymous)::LocalClass"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Defined")
    {
      EXPECT_EQ(std::string("simple.cpp:4:1"), node.value);
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Inherits From")
    {
      std::vector<InfoNode> inherits;
      _cppservice->getSubInfoTree(inherits, localClass.astNodeId, node.query);
      EXPECT_EQ(0, inherits.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Inherited By")
    {
      std::vector<InfoNode> inherited;
      _cppservice->getSubInfoTree(inherited, localClass.astNodeId, node.query);
      EXPECT_EQ(0, inherited.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Friends")
    {
      std::vector<InfoNode> friends;
      _cppservice->getSubInfoTree(friends, localClass.astNodeId, node.query);
      EXPECT_EQ(0, friends.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Methods")
    {
      std::vector<InfoNode> subsets;
      _cppservice->getSubInfoTree(subsets, localClass.astNodeId, node.query);
      ASSERT_EQ(6, subsets.size()); //methods and Inherited
      std::set<std::string> expected{"void", "int", "[ignored]"};
      std::set<std::string> expLabels
      {
        "LocalClass()",
        "~LocalClass()",
        "getPrivX()",
        "setPrivX(int)",
      };
      for (const InfoNode& node : subsets)
      {
        if (node.category.front() == "public")
        {
          EXPECT_TRUE(expected.count(node.value) > 0);
          EXPECT_TRUE(expLabels.count(node.label) > 0);
        }
        else if ( node.category.front() != "Inherited")
        {
          std::cout << node.category.front() << std::endl;
          EXPECT_TRUE(false);
        }

      }
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Members")
    {
      std::vector<InfoNode> subsets;
      _cppservice->getSubInfoTree(subsets, localClass.astNodeId, node.query);
      ASSERT_EQ(2, subsets.size()); //privX, Inherited
      EXPECT_EQ("private", subsets[0].category.front());

      EXPECT_EQ("_privX", subsets[0].label);
      EXPECT_EQ("int", subsets[0].value);

      EXPECT_EQ("Inherited", subsets[1].category.front());

      ++hitcounter;
    }
  }
  EXPECT_EQ(7, hitcounter);
}

TEST_F(CppServiceTest, getDefinitionSimpleImplicit)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto simpleCpp = getFileId("simple.cpp");

  checkDefinition(63, 4, simpleCpp, {52}); // MyStruct s1, s2;
  checkDefinition(68, 4, simpleCpp, {61}); // ImplicitTest b1;
}

TEST_F(CppServiceTest, InfoTreeOfVariableImplicit)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto simpleCpp = getFileId("simple.cpp");
  AstNodeInfo astnodeinfo = getAstNodeInfoByPos(68, 17, simpleCpp);

  std::vector<InfoNode> infonodes;
  _cppservice->getInfoTree(infonodes, astnodeinfo.astNodeId);

  int hitcounter = 0;
  for (const InfoNode& node : infonodes)
  {
    if (node.label == "Name")
    {
      EXPECT_EQ(std::string("b1"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Qualified Name")
    {
      EXPECT_EQ(std::string("b1"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Type")
    {
      EXPECT_EQ(std::string("ImplicitTest"), node.astValue.astNodeValue);
      ++hitcounter;
    }
    else if (node.label == "Declaration")
    {
      EXPECT_EQ(std::string("simple.cpp:68:3"), node.value);
      ++hitcounter;
    }
    // reads and writes are not important here
  }
  EXPECT_EQ(4, hitcounter);
}

TEST_F(CppServiceTest, InfoTreeOfFieldImplicit)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto simpleCpp = getFileId("simple.cpp");
  AstNodeInfo astnodeinfo = getAstNodeInfoByPos(63, 13, simpleCpp);

  std::vector<InfoNode> infonodes;
  _cppservice->getInfoTree(infonodes, astnodeinfo.astNodeId);

  int hitcounter = 0;
  for (const InfoNode& node : infonodes)
  {
    if (node.label == "Name")
    {
      EXPECT_EQ(std::string("s1"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Qualified Name")
    {
      EXPECT_EQ(std::string("ImplicitTest::s1"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Type")
    {
      EXPECT_EQ(std::string("MyStruct"), node.astValue.astNodeValue);
      ++hitcounter;
    }
    else if (node.label == "Declaration")
    {
      EXPECT_EQ(std::string("simple.cpp:63:3"), node.value);
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Reads")
    {
      std::vector<InfoNode> reads;
      _cppservice->getSubInfoTree(reads, astnodeinfo.astNodeId, node.query);
      EXPECT_EQ(0, reads.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Writes")
    {
      std::vector<InfoNode> writes;
      _cppservice->getSubInfoTree(writes, astnodeinfo.astNodeId, node.query);
      EXPECT_EQ(0, writes.size());
      ++hitcounter;
    }
  }
  EXPECT_EQ(6, hitcounter);
}

TEST_F(CppServiceTest, InfoTreeOfTypeImplicit)
{
  init("sqlite:user=parser_integration_test;database=" TOP_BLDDIR "/test/tmp_servicetest/simple/simple.sqlite");
  auto fileId = getFileId("simple.cpp");
  AstNodeInfo astnodeinfo = getAstNodeInfoByPos(61, 11, fileId);

  std::vector<InfoNode> infonodes;
  _cppservice->getInfoTree(infonodes, astnodeinfo.astNodeId);

  int hitcounter = 0;
  for (const InfoNode& node : infonodes)
  {
    if (node.label == "Qualified Name")
    {
      EXPECT_EQ(std::string("ImplicitTest"), node.value);
      ++hitcounter;
    }
    else if (node.label == "Defined")
    {
      EXPECT_EQ(std::string("simple.cpp:61:1"), node.value);
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Inherits From")
    {
      std::vector<InfoNode> inherits;
      _cppservice->getSubInfoTree(inherits, astnodeinfo.astNodeId, node.query);
      EXPECT_EQ(0, inherits.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Inherited By")
    {
      std::vector<InfoNode> inherited;
      _cppservice->getSubInfoTree(inherited, astnodeinfo.astNodeId, node.query);
      EXPECT_EQ(0, inherited.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Friends")
    {
      std::vector<InfoNode> friends;
      _cppservice->getSubInfoTree(friends, astnodeinfo.astNodeId, node.query);
      EXPECT_EQ(0, friends.size());
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Methods")
    {
      std::vector<InfoNode> subsets;
      _cppservice->getSubInfoTree(subsets, astnodeinfo.astNodeId, node.query);
      ASSERT_EQ(5, subsets.size()); // methods and Inherited
      std::set<std::string> expected{"[ignored]", "ImplicitTest &"};
      std::set<std::string> expLabels
      {
        "ImplicitTest()",
        "~ImplicitTest()",
        "ImplicitTest(const ImplicitTest &)",
        "operator=(const ImplicitTest &)",
      };
      for (const InfoNode& node : subsets)
      {
        if (node.category.front() == "Compiler-generated" && node.category.at(1) == "public")
        {
          // TODO: Test for method body generation
          EXPECT_TRUE(expected.count(node.value) > 0);
          EXPECT_TRUE(expLabels.count(node.label) > 0);
        }
        else if ( node.category.front() != "Inherited")
        {
          EXPECT_TRUE(false);
        }

      }
      ++hitcounter;
    }
    else if (!node.category.empty() && node.category.front() == "Members")
    {
      std::vector<InfoNode> subsets;
      _cppservice->getSubInfoTree(subsets, astnodeinfo.astNodeId, node.query);
      ASSERT_EQ(3, subsets.size()); // s1, s2, Inherited
      EXPECT_EQ("public", subsets[0].category.front());

      EXPECT_EQ("s1", subsets[0].label);
      EXPECT_EQ("struct MyStruct", subsets[0].value);
      EXPECT_EQ("s2", subsets[1].label);
      EXPECT_EQ("struct MyStruct", subsets[1].value);

      EXPECT_EQ("Inherited", subsets[2].category.front());

      ++hitcounter;
    }
  }
  EXPECT_EQ(7, hitcounter);
}

// ----------------------------------------------------
// ============================================================================
// ----------------------------------------------------

/**
  * General definition test
  */
TEST_F(CppServiceTest, TestDefinitionCpptest)
{
  init("sqlite:user=parser_integration_test;database="
       TOP_BLDDIR "/test/tmp_servicetest/cpptest/cpptest.sqlite");
  auto cpptest = getFileId("cpptest.cpp");

  checkDefinition(95, 20, cpptest, {52}); // Dog
  checkDefinition(127, 45, cpptest, {127}); // lambda function parameter - i  
  checkDefinition(127, 49, cpptest, {127}); // lambda function parameter - j  
}

/**
  * General class info tree test
  */
TEST_F(CppServiceTest, TestVariableSimpleLocalVar2)
{
  EXPECT_TRUE(checkVariable(
                "simple", // testDir,
                42, // line,
                16, // col,
                "locaVar2", // name,
                "locaVar2", // qname,
                "class (anonymous namespace)::LocalClass", // type,
                "simple.cpp:42:3", // declaration,
                0, // callsNum,
                0, // readsNum,
                "simple.cpp (2)", // writesInFile,
  {"  locaVar2.getPrivX();", "  locaVar2.setPrivX(0);"}, // writeNames,
  {"43:3", "44:3"}// writeLabels,
              ));
}

/**
  * We miss public member _privX, test should fail. (EXPECT_FALSE)
  */
TEST_F(CppServiceTest, TestClassSimpleLocalClass)
{
  EXPECT_FALSE(checkClass(
                 "simple",
                 4,
                 8,
                 "(anonymous)::LocalClass",
                 "simple.cpp:4:1",
                 0,
                 0,
                 0,
  {"LocalClass()", "~LocalClass()", "getPrivX()", "setPrivX(int)"},
  {}
               ));
}

/**
 * "life is a public member of Creature, test if we find it"
 */
TEST_F(CppServiceTest, TestClassCpptest)
{
  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                5, // line
                8, // col
                "Creature", // qname
                "cpptest.cpp:5:1", // defined
                0, // inheritsFromNum
                2, //inheritedByNum
                0, // friendsNum
  {"eat()"}, // publicMethodLabels
  { std::tuple<std::string, std::string, std::string>
    {"public", "int", "life"}
  } // members ( visibilityMod, retVal, name )
              ));


  /**
   * "life" is inherited member, that we dont mention in this test,
   * hence test should fail (expect_false).
   *
   */;
  EXPECT_FALSE(checkClass(
                 "cpptest", // testDir
                 12, // line
                 9, // col
                 "Mammal", // qname
                 "cpptest.cpp:12:1", // defined
                 1, // inheritsFromNum
                 1, //inheritedByNum
                 0, // friendsNum
  {"eat()", "breathe()"}, // publicMethodLabels
  {} // members ( visibilityMod, retVal, name )
               ));

  /**
   * "life is an inherited member of WingedCreature, test if we find it"
   */
  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                17, // line
                9, // col
                "WingedCreature", // qname
                "cpptest.cpp:17:1", // defined
                1, // inheritsFromNum
                2, //inheritedByNum
                0, // friendsNum
  {"eat()", "flap()"}, // publicMethodLabels
  { std::tuple<std::string, std::string, std::string>
    {"public", "int", "life"}
  } // members ( visibilityMod, retVal, name )
              ));

  /**
    * test static member
    */
  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                23, // line
                8, // col
                "Bat", // qname
                "cpptest.cpp:23:1", // defined
                2, // inheritsFromNum
                0, //inheritedByNum
                0, // friendsNum
  {"flap()", "breathe()", "eat()"}, // publicMethodLabels
  { std::tuple<std::string, std::string, std::string>
    {"public", "int", "life"},
    std::tuple<std::string, std::string, std::string> // members ( visibilityMod, retVal, name )
    {"public", "int", "simaInt"},
    std::tuple<std::string, std::string, std::string> // members ( visibilityMod, retVal, name )
    {"Static", "const int", "staticInt"}
  }
              ));

  /**
  * Test inherited method
  */
  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                135, // line
                8, // col
                "Bird", // qname
                "cpptest.cpp:135:1", // defined
                1, // inheritsFromNum
                0, //inheritedByNum
                0, // friendsNum
                // {"flap()", "eat()"}, // publicMethodLabels
  {"flap()", "tweet()", "eat()"}, // publicMethodLabels
  { std::tuple<std::string, std::string, std::string> // members ( visibilityMod, retVal, name )
    {"public", "int", "wing"}
  }
              ));

  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                32, // line
                8, // col
                "Animal", // qname
                "cpptest.cpp:32:1", // defined
                0, // inheritsFromNum
                1, //inheritedByNum
                0, // friendsNum
  {"makeNoise()"}, // publicMethodLabels
  {} // members ( visibilityMod, retVal, name )
              ));

  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                44, // line
                8, // col
                "Dog", // qname
                "cpptest.cpp:44:1", // defined
                1, // inheritsFromNum
                0, //inheritedByNum
                0, // friendsNum
  {"Dog()", "makeNoise()", "newDog()"}, // publicMethodLabels
  {} // members ( visibilityMod, retVal, name )
              ));

  /**
  * Test new operator
  */
  EXPECT_TRUE(checkClass(
                "cpptest", // testDir
                70, // line
                9, // col
                "X", // qname
                "cpptest.cpp:70:1", // defined
                0, // inheritsFromNum
                0, //inheritedByNum
                0, // friendsNum
  {"new(std::size_t)", "new[](std::size_t)"}, // publicMethodLabels
  {} // members ( visibilityMod, retVal, name )
              ));
}

TEST_F(CppServiceTest, TestOverloadedNewOperator)
{
  init("sqlite:user=parser_integration_test;database="
       TOP_BLDDIR "/test/tmp_servicetest/cpptest/cpptest.sqlite");
  auto fid = getFileId("cpptest.cpp");

  checkDefinition(87, 11, fid, {128}); // non-overloaded ne operator in trunk-deps/include/c++/5.1.0/new
  checkDefinition(89, 12, fid, {71}); // overloaded normal new operator of X in test/sources/cpptest/cpptest.cpp
  checkDefinition(91, 12, fid, {76}); // overloaded normal new operator of X in test/sources/cpptest/cpptest.cpp
}

TEST_F(CppServiceTest, TestFunctionCallViaPointer)
{
  init("sqlite:user=parser_integration_test;database="
       TOP_BLDDIR "/test/tmp_servicetest/cpptest/cpptest.sqlite");
  auto fid = getFileId("cpptest.cpp");

  checkDefinition(105, 13, fid, {38, 59}, 2);
  checkDefinition(170, 6, fid, {158, 163}, 2);
  checkDefinition(173, 6, fid, {158}, 1);
  checkDefinition(176, 6, fid, {158, 163}, 2);
}

TEST_F(CppServiceTest, TestVariableCpptest)
{
  /**
  * Test variable pointer info tree
  */
  EXPECT_TRUE(checkVariable(
                "cpptest", // testDir,
                89, // line,
                7, // col,
                "p1", // name,
                "p1", // qname,
                "struct X *", // type,
                "cpptest.cpp:89:3", // declaration,
                0, // callsNum,
                0, // readsNum,
                "cpptest.cpp (1)", // writesInFile,
  {"  delete p1;"}, // writeNames,
  {"90:10"}// writeLabels,

  /**
  * Test variable pointer info tree (pointer to array)
  */      ));
  EXPECT_TRUE(checkVariable(
                "cpptest", // testDir,
                91, // line,
                7, // col,
                "p2", // name,
                "p2", // qname,
                "struct X *", // type,
                "cpptest.cpp:91:3", // declaration,
                0, // callsNum,
                0, // readsNum,
                "cpptest.cpp (1)", // writesInFile,
  {"  delete[] p2;"}, // writeNames,
  {"92:12"}// writeLabels,
              ));

  /**
  * Test pointer with function call
  */
  EXPECT_TRUE(checkVariable(
                "cpptest", // testDir,
                98, // line,
                5, // col,
                "pDog", // name,
                "pDog", // qname,
                "class Dog *", // type,
                "cpptest.cpp:95:3", // declaration,
                0, // callsNum,
                1, // readsNum,
                "", // writesInFile,
                {}, // writeNames,
                {}// writeLabels,
              ));

  /**
   * pBark is a Variable instead of a function pointer.
   * (pDog->*pBark)(); in line 98 should be a 'calls', not a 'read'.
   */
  EXPECT_TRUE(checkVariable(
                "cpptest", // testDir,
                98, // line,
                12, // col,
                "pBark", // name,
                "pBark", // qname,
                "class Dog::BarkFunction", // type,
                "cpptest.cpp:96:3", // declaration,
                1, // callsNum,
                0, // readsNum,
                "", // writesInFile,
                {}, // writeNames,
                {}// writeLabels,
              ));


  /**
  * Test function pointer info tree
  */
  EXPECT_TRUE(checkVariable(
                "cpptest", // testDir,
                103, // line,
                24, // col,
                "pNew", // name,
                "pNew", // qname,
                "class Dog::NewDogFunction",
                "cpptest.cpp:101:3", // declaration,
                1, // callsNum,
                1, // readsNum,
                "", // writesInFile,
                {}, // writeNames,
                {}// writeLabels,
              ));
}

TEST_F(CppServiceTest, TestFunctionCpptest)
{

  /**
  * Test new operator as function
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                89, // line,
                12, // col,
                "operator new", // name,
                "X::operator new", // qname,
                "void * operator new(std::size_t)", // signature
                "void *", // returnType
                "cpptest.cpp:71:3", // defined
                0, // declNum
  {"std::size_t"}, // paramNames
  {}, // localVarNames
  2, // calleesNum
  1,// callersNum
  0// assignedNum
              ));

  /**
  * Test function info tree
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                59, // line,
                17, // col,
                "makeNoise", // name,
                "Dog::makeNoise", // qname,
                "void makeNoise()", // signature
                "void", // returnType
                "cpptest.cpp:59:3", // defined
                0, // declNum
                {}, // paramNames
                {}, // localVarNames
                1, // calleesNum
                2,// callersNum
                1// assignedNum
              ));

  /**
  * Test assign to function pointer
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                54, // line,
                16, // col,
                "newDog", // name,
                "Dog::newDog", // qname,
                "class Dog * newDog()", // signature
                "class Dog *", // returnType
                "cpptest.cpp:54:3", // defined
                0, // declNum
                {}, // paramNames
                {}, // localVarNames
                2, // calleesNum
                1,// callersNum
                1// assignedNum
              ));

  /**
  * Test new operator as function
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                71, // line,
                26, // col,
                "operator new", // name,
                "X::operator new", // qname,
                "void * operator new(std::size_t)", // signature
                "void *", // returnType
                "cpptest.cpp:71:3", // defined
                0, // declNum
  {"std::size_t"}, // paramNames
  {}, // localVarNames
  2, // calleesNum
  1,// callersNum
  0// assignedNum
              ));

  /**
  * Test new operator as function []
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                76, // line,
                26, // col,
                "operator new[]", // name,
                "X::operator new[]", // qname,
                "void * operator new[](std::size_t)", // signature
                "void *", // returnType
                "cpptest.cpp:76:3", // defined
                0, // declNum
  {"std::size_t"}, // paramNames
  {}, // localVarNames
  2, // calleesNum
  1,// callersNum
  0// assignedNum
              ));

  /**
  * Test standard new operator as function
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                79, // line,
                24, // col,
                "operator new", // name,
                "operator new", // qname,
                "void * operator new(unsigned long)", // signature
                "void *", // returnType
                "new:128:1", // defined
                1, // declNum
  {"unsigned long"}, // paramNames
  {}, // localVarNames
  0, // calleesNum
  10,// callersNum
  0// assignedNum
              ));
  
  /**
  * Test standard delete operator as function
  */
  EXPECT_TRUE(checkFunction(
                "cpptest", // testDir,
                90, // line,
                4, // col,
                "operator delete", // name,
                "operator delete", // qname,
                "void operator delete(void *)", // signature
                "void", // returnType
                "new:132:1", // defined
                1, // declNum
  {"void *"}, // paramNames
  {}, // localVarNames
  0, // calleesNum
  3,// callersNum
  0// assignedNum
              ));
}

TEST_F(CppServiceTest, TestInfoBoxCpptest)
{
  /**
  * Infobox definition test
  */
  EXPECT_TRUE(checkInfoBox( // Class
                "cpptest", // testDir,
                5, // line,
                8, // col,
                "class Creature {"
              ));

  /**
    * Infobox definition test
    */
  EXPECT_TRUE(checkInfoBox( // Typedef
                "cpptest", // testDir,
                36, // line,
                21, // col,
                "typedef Animal*(*NewAnimalFunction)(void)"
              ));

  /**
    * Infobox definition test
    */
  EXPECT_TRUE(checkInfoBox( // Fuction pointer
                "cpptest", // testDir,
                101, // line,
                24, // col,
                "Dog::NewDogFunction pNew = &Dog::newDog"
              ));
  /*Tests if infobox correctly displays
   *the definition of a function
  **/
  EXPECT_TRUE(checkInfoBox(
                "cpptest",
                105, // line,
                13, // col,
                "virtual void makeNoise()"
              ));

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
