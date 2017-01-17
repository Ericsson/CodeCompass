#ifndef CC_SERVICE_TEST_SERVICEHELPER_H
#define CC_SERVICE_TEST_SERVICEHELPER_H

#include <odb/database.hxx>

#include <util/odbtransaction.h>

#include <service/cppservice.h>

using namespace cc::service::language;

/**
 * This variable helps us to pass the connection string from the test argument
 * to each test cases.
 */
extern const char* dbConnectionString;

namespace cc
{
namespace service
{
namespace test
{

class ServiceHelper
{
public:
  typedef std::map<std::string, std::vector<int>> ExpectedValues;

  ServiceHelper(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<CppServiceHandler> cppservice_);

  /**
   * Return a file id for the file name.
   */
  model::FileId getFileId(const std::string& fileName_);

  /**
   * Get AST node info by position.
   * @param line_ Line number of the clicked node.
   * @param col_ Column number of the clicked node.
   * @param fid_ File id where it looks the ast node.
   */
  AstNodeInfo getAstNodeInfoByPos(int line_, int col_, model::FileId fid_);

  /**
   * This function returns with reference types for the AST node.
   */
  std::map<std::string, std::int32_t> getReferenceType(
    core::AstNodeId nodeId_);

  /**
   * This function checks the reference of a clicked node. If the reference
   * size isn't equal with the expected line or any expected line number isn't
   * equal with the correct reference's line number, the test will fail.
   */
  void checkReferences(
    int line_, int col_,
    const model::FileId& fid_,
    const ExpectedValues& expectedValues_);

private:
  std::shared_ptr<odb::database> _db;
  std::shared_ptr<CppServiceHandler> _cppservice;
  util::OdbTransaction _transaction;
};

} // test
} // service
} // cc

#endif // CC_SERVICE_TEST_SERVICEHELPER_H
