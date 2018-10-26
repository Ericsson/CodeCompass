#ifndef CC_SERVICE_CPPREPARSESERVICE_REPARSER_H
#define CC_SERVICE_CPPREPARSESERVICE_REPARSER_H

#include <memory>

#include <boost/variant.hpp>

#include <odb/database.hxx>
#include <util/odbtransaction.h>

// Required for the Thrift objects, such as core::FileId.
#include "cppreparse_types.h"

namespace clang
{
class ASTUnit;

namespace tooling
{
class FixedCompilationDatabase;
} // namespace tooling
} // namespace clang

namespace cc
{
namespace service
{
namespace reparse
{

class ASTCache;

class CppReparser
{
public:
  CppReparser(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<ASTCache> astCache_);
  CppReparser(const CppReparser&) = delete;
  CppReparser& operator=(const CppReparser&) = delete;
  ~CppReparser() = default;

  /**
   * Obtain the compilation command that used the given file as source file.
   * @param fileId_ The file ID of the file to query.
   * @return Either a pointer to the compilation database containing the compile
   * command for the File ID, if no errors happen, or a string explaining the
   * error happened.
   */
  boost::variant<std::unique_ptr<clang::tooling::FixedCompilationDatabase>,
                 std::string>
  getCompilationCommandForFile(const core::FileId& fileId_);

  /**
   * Returns the ASTUnit instance for the given source file
   * @param fileId_ The file ID of the file to build the AST for.
   * @return An ASTUnit pointer, on which ASTConsumers can be executed. If an
   * error happened and the AST could not be obtained, a string explaining the
   * error is returned.
   */
  boost::variant<std::shared_ptr<clang::ASTUnit>, std::string>
  getASTForTranslationUnitFile(const core::FileId& fileId_);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::shared_ptr<ASTCache> _astCache;

  std::string getFilenameForId(const core::FileId& fileId_);
};

} // namespace reparse
} // namespace service
} // namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_REPARSER_H
