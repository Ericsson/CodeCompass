#include <iostream>
#include <fstream>

#include <odb/transaction.hxx>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/ADT/Twine.h"

#include <projectparser/generalprojectparser.h>
#include <parser/parser.h>
#include <fileparser/fileparser.h>
#include <searchparser/searchparser.h>

#include <model/workspace.h>
#include <model/file.h>
#include <model/option.h>
#include <model/option-odb.hxx>

#include <util/util.h>
#include <util/odbtransaction.h>
#include "includepath.h"

namespace cc
{
namespace parser
{

GeneralProjectParser::GeneralProjectParser(
  std::shared_ptr<model::Workspace> w_,
  ParseProps& props_,
  SourceManager& srcMgr_)
:
  ProjectParser(w_, props_, srcMgr_),
  _buildIdCounter(0),
  _linkStepReady(false)
{
}

bool GeneralProjectParser::initParse(
  const std::string& path_,
  ProjectParser::ParserContext& context_)
{
  TraverseContext context;
  iterateDirectoryRecursive(context, path_, { [&, this](
    const std::string& filePath_, Traversal::FileType fileType_) -> bool
    {
      if (fileType_ == Traversal::FileType::RegularFile)
      {
        auto parser = FileParser::instance().getParser(filePath_);
        if (parser)
        {
          std::string targetPath = parser->getDefaultTargetPath(filePath_);

          _sourceTargets.emplace_back(std::make_tuple(filePath_, targetPath));
        }
      }

      return true;
    } }
  );

  _props.options["projroot"] = path_;

  context_.roots["source"] = path_;
  context_.partiallyParsed = false;
  context_.sourceFileCount = _sourceTargets.size();

  _sourceTargetsIter = _sourceTargets.begin();
  _linkStepReady = false;

  return true;
}

bool GeneralProjectParser::getNextTask(
  ProjectParser::ParserContext& context_,
  ProjectParser::ParserTask& task_)
{
  if (_linkStepReady)
  {
    return false;
  }
  else if (_sourceTargetsIter == _sourceTargets.end())
  {
    // Link step

    if (_sourceTargets.empty())
    {
      _linkStepReady = true;
      return false;
    }

    std::vector<std::string> targets;
    for (const auto& srcAuto : _sourceTargets)
    {
      targets.push_back(std::get<1>(srcAuto));
    }

    std::string binaryPath = util::getPath(targets[0]) + "binary";

    task_.action = addBuildAction(model::BuildAction::Link, ++_buildIdCounter);
    addBuildTarget(task_.action, binaryPath);
    for(const std::string& file : targets)
    {
      addBuildSource(task_.action, file);
    }

    _linkStepReady = true;
  }
  else
  {
    std::string sourceFile = std::get<0>(*_sourceTargetsIter);
    std::string targetFile = std::get<1>(*_sourceTargetsIter);

    task_.action = addBuildAction(model::BuildAction::Compile,
      ++_buildIdCounter);
    task_.options  = DefaultIncludePathFinder::find(sourceFile);
    if(util::isCppFile(sourceFile)){
      task_.options.push_back("-std=c++11");
    }
    for (const auto& option : task_.options)
    {
      addBuildParameter(task_.action, option);
    }    

    task_.files.push_back(std::make_tuple(
      addBuildSource(task_.action, sourceFile),
      addBuildTarget(task_.action, targetFile)));

    ++_sourceTargetsIter;
  }

  return true;
}

bool GeneralProjectParser::accept(const std::string& path_)
{
  llvm::sys::fs::directory_entry de(path_);
  llvm::sys::fs::file_status status;
  de.status(status);
  return status.type() == llvm::sys::fs::file_type::directory_file;
}

} // parser
} // cc
