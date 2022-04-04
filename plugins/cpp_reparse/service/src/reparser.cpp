#include <boost/algorithm/string.hpp>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <model/builddirectory.h>
#include <model/builddirectory-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/logutil.h>

#include <service/reparser.h>

#include "astcache.h"
#include "databasefilesystem.h"

namespace
{

typedef odb::query<cc::model::BuildDirectory> BuildDirQuery;
typedef odb::query<cc::model::BuildSource> BuildSourceQuery;
typedef odb::result<cc::model::BuildSource> BuildSourceResult;
typedef odb::query<cc::model::File> FileQuery;

} // namespace (anonymous)

namespace cc
{
namespace service
{
namespace reparse
{

using namespace cc::service;
using namespace clang;
using namespace clang::tooling;

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

boost::variant<
  std::unique_ptr<clang::tooling::FixedCompilationDatabase>, std::string>
CppReparser::getCompilationCommandForFile(
  const core::FileId& fileId_)
{
  cc::model::BuildActionPtr buildAction;

  _transaction([&, this](){
    BuildSourceResult bss = _db->query<model::BuildSource>(
      BuildSourceQuery::file == std::stoull(fileId_));
    for (auto bs : bss)
      if (!buildAction || buildAction->command.empty())
        buildAction = bs.action;
      else
      {
        LOG(warning) << "Multiple build commands present for source file #"
                     << std::stoull(fileId_) << ". Considering first result.";
        /*break;*/

        LOG(debug) << "Another build command could be: ";
        LOG(debug) << bs.action->command;
      }
  });

  if (!buildAction)
  {
    return "Build command not found for the file! Is this not a C++ file, "
      "or a header?";
  }

  std::string buildDir;

  _transaction([&, this](){
    cc::model::BuildDirectoryPtr bd = _db->query_one<model::BuildDirectory>(
      BuildDirQuery::action == buildAction->id);
    if (bd)
      buildDir = bd->directory;
    else
    {
      LOG(debug) << "No build directory for source file #"
                 << std::stoull(fileId_) << ". Using '/'.";
      buildDir = "/";
    }
  });

  //--- Assemble compiler command line ---//

  std::vector<const char*> commandLine;
  std::vector<std::string> split;
  boost::split(split, buildAction->command, boost::is_any_of(" "));

  commandLine.reserve(split.size());
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
      compilationDbLoadError,
      buildDir));

  if (!compilationDb)
  {
    return "Failed to create compilation database from build action: " +
           compilationDbLoadError;
  }

  return std::move(compilationDb);
}

boost::variant<std::shared_ptr<clang::ASTUnit>, std::string>
CppReparser::getASTForTranslationUnitFile(
  const core::FileId& fileId_)
{
  std::shared_ptr<ASTUnit> AST = _astCache->getAST(fileId_);
  if (!AST)
  {
    LOG(debug) << "Fetching AST for " << fileId_ << " from database...";

    auto compilation = getCompilationCommandForFile(fileId_);
    if (std::string* err = boost::get<std::string>(&compilation))
    {
      return "Failed to generate compilation command for file #" +
        std::to_string(std::stoull(fileId_)) + ": " + *err;
    }

    // TODO: FIXME: Change this into the shortcutting overlay creation once the interface is upstreamed. (https://reviews.llvm.org/D45094)
    IntrusiveRefCntPtr<DatabaseFileSystem> dbfs(
      new DatabaseFileSystem(_db));
    IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> overlayFs(
      new llvm::vfs::OverlayFileSystem(
        llvm::vfs::createPhysicalFileSystem().release()));
    overlayFs->pushOverlay(dbfs);

    auto compileDb = std::move(
      boost::get<std::unique_ptr<clang::tooling::FixedCompilationDatabase>>(
        compilation));

    ClangTool tool(
      *compileDb, getFilenameForId(fileId_),
      std::make_shared<clang::PCHContainerOperations>(), overlayFs);

    std::vector<std::unique_ptr<ASTUnit>> vect;
    int error = tool.buildASTs(vect);
    if (error)
    {
      return "Execution of parsing the AST failed with error code " +
        std::to_string(error);
    }

    AST = _astCache->storeAST(fileId_, std::move(vect.at(0)));
  }

  return AST;
}

} // namespace reparse
} // namespace service
} // namespace cc
