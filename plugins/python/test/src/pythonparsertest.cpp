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
    loadFile("imports.py");
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

    file = _db->query_value<model::File>(odb::query<model::File>::filename == "classes.py");
    EXPECT_EQ(file.type, "PY");
    EXPECT_EQ(file.parseStatus, model::File::PSFullyParsed);

    file = _db->query_value<model::File>(odb::query<model::File>::filename == "imports.py");
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

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 4 &&
                                           odb::query<model::PYName>::column_start == 12);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 4 &&
                                           odb::query<model::PYName>::column_start == 18);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 4 &&
                                           odb::query<model::PYName>::column_start == 26);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 7 &&
                                           odb::query<model::PYName>::column_start == 9);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 7 &&
                                           odb::query<model::PYName>::column_start == 12);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 10 &&
                                           odb::query<model::PYName>::column_start == 10);
  EXPECT_EQ(pyname.type, "astparam");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 10 &&
                                           odb::query<model::PYName>::column_start == 13);
  EXPECT_EQ(pyname.type, "astparam");
}

TEST_F(PythonParserTest, FunctionSignatureAST)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 49 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 52 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign2(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 57 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign3(a: int, b: str) -> None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 62 &&
                                           odb::query<model::PYName>::type == "function");
  EXPECT_EQ(pyname.value, "def sign4(a: int, b: str) -> None");
}

TEST_F(PythonParserTest, FunctionAnnotationAST)
{
  model::PYName pyname;

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 69 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "None");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 73 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "str");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 77 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "int");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 81 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "bool");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 85 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "List[str]");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 89 &&
                                           odb::query<model::PYName>::type == "annotation");
  EXPECT_EQ(pyname.value, "Optional[str]");

  pyname = queryFile("functions.py", odb::query<model::PYName>::line_start == 93 &&
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
                                           odb::query<model::PYName>::line_start == 11);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 14);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 17);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 22);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 26);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 27);
  EXPECT_EQ(pyname.is_call, false);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 43);
  EXPECT_EQ(pyname.is_call, true);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 44);
  EXPECT_EQ(pyname.is_call, false);
}

TEST_F(PythonParserTest, ClassType)
{
  model::PYName pyname;

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 1 &&
        odb::query<model::PYName>::value == "class Base:\n"));

  EXPECT_EQ(pyname.type, "class");

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 19 &&
        odb::query<model::PYName>::value == "class Derived(Base):\n"));

  EXPECT_EQ(pyname.type, "class");

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 23 &&
        odb::query<model::PYName>::value == "class Derived2(Derived, Base):\n"));

  EXPECT_EQ(pyname.type, "class");
}

TEST_F(PythonParserTest, ClassInheritance)
{
  model::PYName pyname;

  model::PYName derived = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 19 &&
        odb::query<model::PYName>::type == "class"));

  model::PYName derived2 = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 23 &&
        odb::query<model::PYName>::type == "class"));

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 19 &&
        odb::query<model::PYName>::column_start == 15 &&
        odb::query<model::PYName>::value == "Base"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived.id);

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 23 &&
        odb::query<model::PYName>::column_start == 16 &&
        odb::query<model::PYName>::value == "Derived"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived2.id);

  pyname = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 23 &&
        odb::query<model::PYName>::column_start == 25 &&
        odb::query<model::PYName>::value == "Base"));

  EXPECT_EQ(pyname.type, "baseclass");
  EXPECT_EQ(pyname.parent, derived2.id);
}

TEST_F(PythonParserTest, ClassMethod)
{
  model::PYName base = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 1 &&
        odb::query<model::PYName>::type == "class"));

  model::PYName foo = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 8 &&
        odb::query<model::PYName>::is_definition == true &&
        odb::query<model::PYName>::type == "function"));

  EXPECT_EQ(foo.parent, base.id);

  model::PYName bar = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 11 &&
        odb::query<model::PYName>::is_definition == true &&
        odb::query<model::PYName>::type == "function"));

  EXPECT_EQ(bar.parent, base.id);

  model::PYName test = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 14 &&
        odb::query<model::PYName>::is_definition == true &&
        odb::query<model::PYName>::type == "function"));

  EXPECT_EQ(test.parent, bar.id);
}

TEST_F(PythonParserTest, ClassDataMember)
{
  model::PYName pyname;

  model::PYName base = queryFile("classes.py",
        (odb::query<model::PYName>::line_start == 1 &&
        odb::query<model::PYName>::type == "class"));

  pyname = queryFile("classes.py",
        odb::query<model::PYName>::value == "DEBUG_MODE = False");

  EXPECT_EQ(pyname.type, "statement");
  EXPECT_EQ(pyname.parent, base.id);

  pyname = queryFile("classes.py",
        odb::query<model::PYName>::value == "users = []");

  EXPECT_EQ(pyname.type, "statement");
  EXPECT_EQ(pyname.parent, base.id);
}

TEST_F(PythonParserTest, LocalVariable)
{
  model::PYName pyname;

  model::PYName func = queryFile("functions.py",
        odb::query<model::PYName>::value == "def local_var()");

  pyname = queryFile("functions.py",
        (odb::query<model::PYName>::line_start == 97 &&
        odb::query<model::PYName>::value == "a = 2"));

  EXPECT_EQ(pyname.type, "statement");
  EXPECT_EQ(pyname.is_definition, true);
  EXPECT_EQ(pyname.parent, func.id);

  pyname = queryFile("functions.py",
        (odb::query<model::PYName>::line_start == 98 &&
        odb::query<model::PYName>::value == "for i in range(0,10):\n"));

  EXPECT_EQ(pyname.type, "statement");
  EXPECT_EQ(pyname.is_definition, true);
  EXPECT_EQ(pyname.parent, func.id);
}

TEST_F(PythonParserTest, ImportModule)
{
  model::PYName pyname;

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 1 &&
        odb::query<model::PYName>::value == "import classes"));

  EXPECT_EQ(pyname.is_import, true);

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 2 &&
        odb::query<model::PYName>::value == "import os"));

  EXPECT_EQ(pyname.is_import, true);

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 3 &&
        odb::query<model::PYName>::value == "from functions import mul"));

  EXPECT_EQ(pyname.is_import, true);
}

TEST_F(PythonParserTest, BuiltinVariable)
{
  model::PYName pyname;

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 2 &&
        odb::query<model::PYName>::value == "import os"));

  EXPECT_EQ(pyname.is_builtin, true);

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 6 &&
        odb::query<model::PYName>::value == "print"));

  EXPECT_EQ(pyname.is_builtin, true);

  pyname = queryFile("imports.py",
        (odb::query<model::PYName>::line_start == 12 &&
        odb::query<model::PYName>::value == "getpid"));

  EXPECT_EQ(pyname.is_builtin, true);

  pyname = queryFile("functions.py",
        (odb::query<model::PYName>::line_start == 85 &&
        odb::query<model::PYName>::value == "str"));

  EXPECT_EQ(pyname.is_builtin, true);

  pyname = queryFile("functions.py",
        (odb::query<model::PYName>::line_start == 85 &&
        odb::query<model::PYName>::value == "List"));

  EXPECT_EQ(pyname.is_builtin, true);

  pyname = queryFile("functions.py",
        (odb::query<model::PYName>::line_start == 98 &&
        odb::query<model::PYName>::value == "range"));

  EXPECT_EQ(pyname.is_builtin, true);
}

TEST_F(PythonParserTest, ReferenceID)
{
  model::PYName pyname;

  model::PYName func = queryFile("functions.py", odb::query<model::PYName>::value == "def mul(a, b)");

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 11);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 14);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 17);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 22);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 26);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 27);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 43);
  EXPECT_EQ(pyname.ref_id, func.id);

  pyname = queryFile("functions.py", odb::query<model::PYName>::value == "mul" &&
                                           odb::query<model::PYName>::line_start == 44);
  EXPECT_EQ(pyname.ref_id, func.id);
}
