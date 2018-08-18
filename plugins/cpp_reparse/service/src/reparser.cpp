#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/hash.h>
#include <util/logutil.h>

#include <service/reparser.h>

#include "astcache.h"
#include "databasefilesystem.h"

namespace
{

using namespace clang;
using namespace clang::tooling;
using namespace cc::service;
using namespace cc::service::reparse;

typedef odb::query<cc::model::BuildSource> BuildSourceQuery;
typedef odb::result<cc::model::BuildSource> BuildSourceResult;
typedef odb::query<cc::model::File> FileQuery;

ClangTool getToolForCompilation(
  const FixedCompilationDatabase& compileDb_,
  std::shared_ptr<odb::database> db_,
  const std::string& filename_)
{
  // TODO: FIXME: Change this into the shortcutting overlay creation once the interface is upstreamed. (https://reviews.llvm.org/D45094)
  IntrusiveRefCntPtr<DatabaseFileSystem> dbfs(new DatabaseFileSystem(db_));
  IntrusiveRefCntPtr<vfs::OverlayFileSystem> overlayFs(
    new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
  overlayFs->pushOverlay(dbfs);

  return ClangTool(compileDb_, filename_,
    std::make_shared<clang::PCHContainerOperations>(), overlayFs);
}

} // namespace (anonymous)

namespace cc
{
namespace service
{
namespace reparse
{

CppReparser::CppReparser(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<ASTCache> astCache_)
  : _db(db_),
    _transaction(db_),
    _astCache(astCache_)
{}

std::string CppReparser::getFilenameForId(const core::FileId& fileId_)
{
  std::string fileName;

  _transaction([&, this](){
    model::FilePtr res = _db->query_one<model::File>(
      FileQuery::id == std::stoull(fileId_));

    if (res)
      fileName = res->path;
  });

  return fileName;
}

CppReparser::CompilationDatabaseOrError
CppReparser::getCompilationCommandForFile(
  const core::FileId& fileId_)
{
  std::string buildCommand;

  _transaction([&, this](){
    BuildSourceResult bss = _db->query<model::BuildSource>(
      BuildSourceQuery::file == std::stoull(fileId_));
    for (auto bs : bss)
      if (buildCommand.empty())
        buildCommand = bs.action->command;
      else
      {
        LOG(warning) << "Multiple build commands present for source file #"
                     << std::stoull(fileId_) << ". Considering first result.";
        /*break;*/

        LOG(debug) << "Another build command could be: ";
        LOG(debug) << bs.action->command;
      }
  });

  if (buildCommand.empty())
  {
    return "Build command not found for the file! Is this not a C++ file, "
      "or a header?";
  }

  std::vector<const char*> commandLine;
  std::vector<std::string> split;
  boost::split(split, buildCommand, boost::is_any_of(" "));

  commandLine.reserve(split.size() + 1);
  commandLine.push_back("--");

  // The build command must be filtered so that certain options, such as
  // dependency files that don't exist anymore as we are not doing an actual
  // parse of the build directory, are omitted.
  size_t numArgsToKeepSkipping = 0;
  for (const auto& it : split)
  {
    if (numArgsToKeepSkipping > 0)
    {
      --numArgsToKeepSkipping;
      continue;
    }

    if (it == "-MM" || it == "-MP" || it == "-MD" || it == "-MV"
        || it == "-MMD")
      // Make dependency options that take no arguments.
      continue;
    if (it == "-MT" || it == "-MQ" || it == "-MF" || it == "-MJ")
    {
      // Make dependency arguments that take an extra file argument.
      // (These files won't be found by the Clang infrastructure at this
      // point...)
      numArgsToKeepSkipping = 1;
      continue;
    }

    commandLine.push_back(it.c_str());
  }

  int argc = commandLine.size();

  std::string compilationDbLoadError;
  std::unique_ptr<FixedCompilationDatabase> compilationDb(
    FixedCompilationDatabase::loadFromCommandLine(
      argc,
      commandLine.data(),
      compilationDbLoadError));

  if (!compilationDb)
    return "Failed to create compilation database from build action: " +
      compilationDbLoadError;

  return std::move(compilationDb);
}

CppReparser::ASTOrError
CppReparser::getASTForTranslationUnitFile(
  const core::FileId& fileId_)
{
  std::shared_ptr<ASTUnit> AST = _astCache->getAST(fileId_);
  if (!AST)
  {
    LOG(debug) << "Fetching AST for " << fileId_ << " from database...";

    auto compilation = getCompilationCommandForFile(fileId_);
    if (std::string* err = boost::get<std::string>(&compilation))
      return "Failed to generate compilation command for file #" +
        std::to_string(std::stoull(fileId_)) + ": " + *err;

    auto compileDb = std::move(boost::get<
      std::unique_ptr<FixedCompilationDatabase>>(compilation));
    ClangTool tool = getToolForCompilation(*compileDb,
      _db, getFilenameForId(fileId_));

    std::vector<std::unique_ptr<ASTUnit>> vect;
    int error = tool.buildASTs(vect);
    if (error)
      return "Execution of parsing the AST failed with error code " +
        std::to_string(error);

    AST = _astCache->storeAST(fileId_, std::move(vect.at(0)));
  }

  return AST;
}

CppReparser::ASTOrError
CppReparser::getASTForString(
  std::string content_,
  const core::FileId& fileIdForCompileReuse_,
  bool prefixOriginalFile_,
  clang::DiagnosticConsumer* diagnostic_)
{
  auto compilation = getCompilationCommandForFile(fileIdForCompileReuse_);
  if (std::string* err = boost::get<std::string>(&compilation))
    return "Failed to generate compilation command for file #" +
      std::to_string(std::stoull(fileIdForCompileReuse_)) + ": " + *err;

  // Create a temporary file in memory with the contents.
  std::string reusedFilename = getFilenameForId(fileIdForCompileReuse_);

  if (prefixOriginalFile_)
    content_ = std::string("#include \"").append(reusedFilename).append("\"\n")
      .append(content_);

  std::string tempFilename = std::string("/tmp/x")
    .append(util::sha1Hash(content_).substr(0, 7))
    .append(boost::filesystem::path(reusedFilename).extension().string());

  // Create a new compile command to indicate the compilation of the temp file.
  auto compileDb = std::move(
    boost::get<std::unique_ptr<FixedCompilationDatabase>>(compilation));
  std::vector<CompileCommand> originalCompCommands =
    compileDb->getCompileCommands(reusedFilename);
  if (originalCompCommands.empty())
    return "Failed to find the compilation command for file " + reusedFilename;
  const CompileCommand& originalCompCommand = originalCompCommands.front();

  int len = originalCompCommand.CommandLine.size() + 1;
  std::vector<const char*> commandLine;
  commandLine.reserve(len);
  commandLine.push_back("--");

  std::transform(originalCompCommand.CommandLine.begin(),
    originalCompCommand.CommandLine.end(),
    std::back_inserter(commandLine),
    [&tempFilename, &reusedFilename] (const std::string& str) {
      if (str == reusedFilename)
        return tempFilename.c_str();
      return str.c_str();
  });
  std::string compilationDbLoadError;
  std::unique_ptr<FixedCompilationDatabase> compilationDb(
    FixedCompilationDatabase::loadFromCommandLine(
      len,
      commandLine.data(),
      compilationDbLoadError));

  if (!compilationDb)
    return "Failed to create compilation database for temporary file: " +
           compilationDbLoadError;

  ClangTool tool = getToolForCompilation(*compilationDb,
    _db, tempFilename);
  tool.mapVirtualFile(tempFilename, content_);
  tool.setDiagnosticConsumer(diagnostic_);

  std::vector<std::unique_ptr<ASTUnit>> vect;
  int error = tool.buildASTs(vect);
  if (error)
    return "Execution of parsing the AST failed with error code " +
      std::to_string(error);

  return std::shared_ptr<ASTUnit>(std::move(vect.at(0)));
}

} // namespace reparse
} // namespace service
} // namespace cc
