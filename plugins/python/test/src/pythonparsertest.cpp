#include <unordered_map>
#include <gtest/gtest.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>

#include <odb/query.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/pyname.h>
#include <model/pyname-odb.hxx>

using namespace cc;
extern const char* dbConnectionString;

class PythonParserTest : public ::testing::Test
{
public:
  PythonParserTest() :
    _db(util::connectDatabase(dbConnectionString)),
    _transaction(_db)
  {
    loadFile("functions.py");
    loadFile("classes.py");
  }

  model::PYName queryFile(const std::string& filename, const odb::query<model::PYName>& odb_query)
  {
    model::PYName pyname;
    if (m_files.count(filename))
    {
      _transaction([&, this]() {
        pyname = _db->query_value<model::PYName>(odb_query && odb::query<model::PYName>::file_id == m_files[filename]);
      });
    }

    return pyname;
  }

private:
  std::unordered_map<std::string, std::uint64_t> m_files;
  void loadFile(const std::string& filename)
  {
    _transaction([&, this]() {
      model::File file = _db->query_value<model::File>(odb::query<model::File>::filename == filename);
      m_files.emplace(filename, file.id);
    });
  }

protected:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
};

TEST_F(PythonParserTest, FilesAreInDatabase)
{
  _transaction([&, this]() {
    model::File file;

    file = _db->query_value<model::File>(odb::query<model::File>::filename == "functions.py");
    EXPECT_EQ(file.type, "PY");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);
  });
}

TEST_F(PythonParserTest, FunctionDefinition)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def hello_world()");
  EXPECT_EQ(pyname.is_definition, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def runner(func, param1, param2)");
  EXPECT_EQ(pyname.is_definition, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul(a, b)");
  EXPECT_EQ(pyname.is_definition, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul2(a, b)");
  EXPECT_EQ(pyname.is_definition, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul3()");
  EXPECT_EQ(pyname.is_definition, true);
}

TEST_F(PythonParserTest, FunctionType)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def hello_world()");
  EXPECT_EQ(pyname.type, "function");

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def runner(func, param1, param2)");
  EXPECT_EQ(pyname.type, "function");

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul(a, b)");
  EXPECT_EQ(pyname.type, "function");

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul2(a, b)");
  EXPECT_EQ(pyname.type, "function");

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul3()");
  EXPECT_EQ(pyname.type, "function");
}

TEST_F(PythonParserTest, FunctionParamAST)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 6 &&
                                           odb::query<model::PYName>::column_start == 12);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 6 &&
                                           odb::query<model::PYName>::column_start == 18);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 6 &&
                                           odb::query<model::PYName>::column_start == 26);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 9 &&
                                           odb::query<model::PYName>::column_start == 9);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 9 &&
                                           odb::query<model::PYName>::column_start == 12);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 12 &&
                                           odb::query<model::PYName>::column_start == 10);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 12 &&
                                           odb::query<model::PYName>::column_start == 13);
  EXPECT_EQ(pyname.type, "astparam");
}

TEST_F(PythonParserTest, FunctionSignatureAST)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 48 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 51 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign2(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 56 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign3(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 61 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign4(a: int, b: str) -> None");
}

TEST_F(PythonParserTest, FunctionAnnotationAST)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 68 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 72 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "str");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 76 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "int");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 80 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "bool");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 84 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "List[str]");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 88 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "Optional[str]");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 92 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "dict[int, bool]");
}

TEST_F(PythonParserTest, FunctionCall)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def hello_world()");
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def runner(func, param1, param2)");
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul(a, b)");
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul2(a, b)");
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "def mul3()");
  EXPECT_EQ(pyname.is_call, false);

  // ----------

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 13);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 16);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 19);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 24);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 28);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 29);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 42);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 43);
  EXPECT_EQ(pyname.is_call, false);
}

TEST_F(PythonParserTest, ClassInheritance)
{
  model::PYName pyname;

  model::PYName base = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 1 &&
        odb::query<model::PYName>::type == "class"));

  model::PYName derived = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 17 &&
        odb::query<model::PYName>::type == "class"));

  model::PYName derived2 = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 21 &&
        odb::query<model::PYName>::type == "class"));

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 17 &&
        odb::query<model::PYName>::column_start == 15 &&
        odb::query<model::PYName>::value == "Base"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived.id);

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 21 &&
        odb::query<model::PYName>::column_start == 16 &&
        odb::query<model::PYName>::value == "Derived"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived2.id);

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 21 &&
        odb::query<model::PYName>::column_start == 25 &&
        odb::query<model::PYName>::value == "Base"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived2.id);
}
