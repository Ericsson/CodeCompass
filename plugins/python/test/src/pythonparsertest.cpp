#include <gtest/gtest.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>

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

TEST_F(PythonParserTest, FunctionDefinitions)
{
  _transaction([&, this]() {
    model::PYName pyname;

    pyname = _db->query_value<model::PYName>(odb::query<model::PYName>::value == "def hello_world()");
    EXPECT_EQ(pyname.is_definition, true);
  });
}
