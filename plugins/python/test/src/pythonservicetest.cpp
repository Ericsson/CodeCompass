#include <gtest/gtest.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <service/pythonservice.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/position.h>

using namespace cc;
using namespace cc::service::language;
extern const char* dbConnectionString;

class PythonServiceTest : public ::testing::Test
{
public:
  PythonServiceTest() :
    _db(util::connectDatabase(dbConnectionString)),
    _transaction(_db),
    _pythonservice(new PythonServiceHandler(
      _db,
      std::make_shared<std::string>(),
      webserver::ServerContext(std::string(), boost::program_options::variables_map()))
    )
  {
    loadFile("functions.py");
    loadFile("classes.py");
    loadFile("imports.py");
  }

  AstNodeInfo getAstNodeInfoByPosition(const std::string& filename, int32_t line, int32_t column)
  {
    AstNodeInfo nodeInfo;

    if (m_files.count(filename) == 0)
    {
      return nodeInfo;
    }

    model::FileId file_id = m_files[filename];

    service::core::FilePosition filePos;
    filePos.file = std::to_string(file_id);
    filePos.pos.line = line;
    filePos.pos.column = column;

    _pythonservice->getAstNodeInfoByPosition(nodeInfo, filePos);
    return nodeInfo;
  }

  size_t referenceFinder(const std::vector<AstNodeInfo>& references,
                         const std::vector<model::Position>& positions)
  {
    size_t c = 0;
    for (const AstNodeInfo& info : references)
    {
      size_t line = info.range.range.startpos.line;
      size_t column = info.range.range.startpos.column;

      for (const model::Position& pos : positions)
      {
        if (pos.line == line && pos.column == column) c++;
      }
    }

    return c;
  }

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::unordered_map<std::string, std::uint64_t> m_files;
  void loadFile(const std::string& filename)
  {
    _transaction([&, this]() {
      model::File file = _db->query_value<model::File>(odb::query<model::File>::filename == filename);
      m_files.emplace(filename, file.id);
    });
  }

protected:
  std::shared_ptr<PythonServiceHandler> _pythonservice;
};

TEST_F(PythonServiceTest, AstNodeInfoByPosition)
{
  AstNodeInfo nodeInfo;

  // Simulating click on line 1 column 5
  nodeInfo = getAstNodeInfoByPosition("functions.py", 1, 5);

  EXPECT_EQ(nodeInfo.astNodeValue, "def hello_world()");
  EXPECT_EQ(nodeInfo.symbolType, "function");

  nodeInfo = getAstNodeInfoByPosition("functions.py", 14, 12);

  EXPECT_EQ(nodeInfo.astNodeValue, "mul");
  EXPECT_EQ(nodeInfo.symbolType, "statement");

  nodeInfo = getAstNodeInfoByPosition("functions.py", 24, 10);

  EXPECT_EQ(nodeInfo.astNodeValue, "MyLib");
  EXPECT_EQ(nodeInfo.symbolType, "statement");
}

TEST_F(PythonServiceTest, NodeProperties)
{
  AstNodeInfo nodeInfo;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 7, 5);
  std::map<std::string, std::string> map;

  _pythonservice->getProperties(map, nodeInfo.id);
  EXPECT_EQ(nodeInfo.astNodeValue, "def mul(a, b)");

  EXPECT_EQ(map.count("Full name"), 1);
  EXPECT_EQ(map["Full name"], "functions.mul");
  EXPECT_EQ(map.count("Builtin"), 1);
  EXPECT_EQ(map["Builtin"], "false");
  EXPECT_EQ(map.count("Function call"), 1);
  EXPECT_EQ(map["Function call"], "false");
}

TEST_F(PythonServiceTest, NodePropertiesBuiltinCall)
{
  AstNodeInfo nodeInfo;

  nodeInfo = getAstNodeInfoByPosition("imports.py", 12, 17);
  EXPECT_EQ(nodeInfo.astNodeValue, "getpid");

  std::map<std::string, std::string> map;

  _pythonservice->getProperties(map, nodeInfo.id);
  EXPECT_EQ(map.count("Full name"), 1);
  EXPECT_EQ(map["Full name"], "imports.getpid");
  EXPECT_EQ(map.count("Builtin"), 1);
  EXPECT_EQ(map["Builtin"], "true");
  EXPECT_EQ(map.count("Function call"), 1);
  EXPECT_EQ(map["Function call"], "true");
}

TEST_F(PythonServiceTest, NodeDefinition)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 11, 12);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::DEFINITION, {});

  EXPECT_EQ(references.size(), 1);
  EXPECT_EQ(references[0].symbolType, "function");
  EXPECT_EQ(references[0].astNodeValue, "def mul(a, b)");

  references = {};
  nodeInfo = getAstNodeInfoByPosition("functions.py", 29, 6);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::DEFINITION, {});

  EXPECT_EQ(references.size(), 1);
  EXPECT_EQ(references[0].symbolType, "statement");
  EXPECT_EQ(references[0].astNodeValue, "mylib = {\n");

  references = {};
  nodeInfo = getAstNodeInfoByPosition("classes.py", 27, 8);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::DEFINITION, {});

  EXPECT_EQ(references.size(), 1);
  EXPECT_EQ(references[0].symbolType, "class");
  EXPECT_EQ(references[0].astNodeValue, "class Base:\n");
}

TEST_F(PythonServiceTest, NodeUsage)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 16, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::USAGE, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "mylib = {\n");
  EXPECT_EQ(references.size(), 2);

  size_t found = referenceFinder(references, {{29, 6}, {46, 11}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeDataMembers)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("classes.py", 1, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::DATA_MEMBER, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "class Base:\n");
  EXPECT_EQ(references.size(), 2);

  size_t found = referenceFinder(references, {{2, 5}, {3, 5}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeLocalVariables)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 96, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::LOCAL_VAR, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "def local_var()");
  EXPECT_EQ(references.size(), 3);

  size_t found = referenceFinder(references, {{97, 5}, {98, 5}, {99, 9}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeParent)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 97, 5);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::PARENT, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "a = 2");
  EXPECT_EQ(references.size(), 1);

  size_t found = referenceFinder(references, {{96, 1}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeParentFunction)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("classes.py", 31, 9);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::PARENT_FUNCTION, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "z = 2");
  EXPECT_EQ(references.size(), 1);

  size_t found = referenceFinder(references, {{29, 1}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeCaller)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 13, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::CALLER, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "def mul3()");
  EXPECT_EQ(references.size(), 2);

  size_t found = referenceFinder(references, {{28, 6}, {45, 11}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeThisCalls)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 10, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::THIS_CALLS, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "def mul2(a, b)");
  EXPECT_EQ(references.size(), 1);

  size_t found = referenceFinder(references, {{11, 12}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeMethod)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("classes.py", 1, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::METHOD, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "class Base:\n");
  EXPECT_EQ(references.size(), 3);

  size_t found = referenceFinder(references, {{5, 5}, {8, 5}, {11, 5}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeBaseClass)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("classes.py", 19, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::BASE_CLASS, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "class Derived(Base):\n");
  EXPECT_EQ(references.size(), 1);

  size_t found = referenceFinder(references, {{1, 1}});
  EXPECT_EQ(found, references.size());

  // ----------

  references = {};
  nodeInfo = getAstNodeInfoByPosition("classes.py", 23, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::BASE_CLASS, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "class Derived2(Derived, Base):\n");
  EXPECT_EQ(references.size(), 2);

  found = referenceFinder(references, {{1, 1}, {19, 1}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeFunctionParam)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 4, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::PARAMETER, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "def runner(func, param1, param2)");
  EXPECT_EQ(references.size(), 3);

  size_t found = referenceFinder(references, {{4, 12}, {4, 18}, {4, 26}});
  EXPECT_EQ(found, references.size());
}

TEST_F(PythonServiceTest, NodeAnnotation)
{
  AstNodeInfo nodeInfo;
  std::vector<AstNodeInfo> references;

  nodeInfo = getAstNodeInfoByPosition("functions.py", 69, 1);
  _pythonservice->getReferences(references, nodeInfo.id, PythonServiceHandler::ANNOTATION, {});

  EXPECT_EQ(nodeInfo.astNodeValue, "def annotation(a, b) -> None");
  EXPECT_EQ(references.size(), 1);

  size_t found = referenceFinder(references, {{69, 24}});
  EXPECT_EQ(found, references.size());
}
