#include <gtest/gtest.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <service/pythonservice.h>

#include <model/file.h>
#include <model/file-odb.hxx>

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
  }

  model::File queryFile(const std::string& filename)
  {
    return _transaction([&, this]() {
      return _db->query_value<model::File>(odb::query<model::File>::filename == filename);
    });
  }

  AstNodeInfo getAstNodeInfoByPosition(const std::string& filename, int32_t line, int32_t column)
  {
    model::File file = queryFile(filename);

    service::core::FilePosition filePos;
    filePos.file = std::to_string(file.id);
    filePos.pos.line = line;
    filePos.pos.column = column;

    AstNodeInfo nodeInfo;
    _pythonservice->getAstNodeInfoByPosition(nodeInfo, filePos);
    return nodeInfo;
  }

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

protected:
  std::shared_ptr<PythonServiceHandler> _pythonservice;
};

TEST_F(PythonServiceTest, AstNodeInfoByPosition)
{
  // Simulating click on line 1 column 5
  AstNodeInfo nodeInfo = getAstNodeInfoByPosition("functions.py", 1, 5);

  EXPECT_EQ(nodeInfo.astNodeValue, "def hello_world()");
  EXPECT_EQ(nodeInfo.symbolType, "function");
}
