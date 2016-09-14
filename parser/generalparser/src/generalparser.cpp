#include <generalparser/generalparser.h>
#include <generalparser/includepath.h>
#include <parser/actionprocessor.h>

#include <model/workspace.h>
#include <model/option.h>
#include <model/option-odb.hxx>
#include <model/buildaction-odb.hxx>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/ADT/Twine.h"

#include <util/util.h>
#include <memory>
#include <iostream>

namespace cc
{
namespace parser
{

std::string GeneralParser::getName()
{
  return "generalparser";
}

std::vector<std::string> GeneralParser::getDependentParsers()
{
  return std::vector<std::string>{};
}
  
bool GeneralParser::accept(const std::string& path_)
{
  llvm::sys::fs::directory_entry de(path_);
  llvm::sys::fs::file_status status;
  de.status(status);
  return status.type() == llvm::sys::fs::file_type::directory_file;
}

bool GeneralParser::initParse(const std::string& path_, Context& context_)
{
  TraverseContext context;
  iterateDirectoryRecursive(context, path_, { [&, this](
    const std::string& filePath_, Traversal::FileType fileType_) -> bool
    {
      if (fileType_ == Traversal::FileType::RegularFile)
      {
//        auto parser = FileParser::instance().getParser(filePath_);
//        if (parser)
//        {
//          std::string targetPath = parser->getDefaultTargetPath(filePath_);
//
//          _sourceTargets.emplace_back(std::make_tuple(filePath_, targetPath));
//        }
      }

      return true;
    } }
  );

  context_.partiallyParsed = false;
  context_.sourceFileCount = _sourceTargets.size();

  _sourceTargetsIter = _sourceTargets.begin();
  _linkStepReady = false;

  return true;
}
bool GeneralParser::parse(const std::vector<std::string>& path_, const ParserContext& ctx_)
{
  _projPath = path_;
  for(const auto& path : path_)
  {
    if(accept(path))
    {
      llvm::sys::fs::directory_entry de(path);
      llvm::sys::fs::file_status status;

      de.status(status);

      if (status.type() == llvm::sys::fs::file_type::file_not_found)
      {
        std::cerr << "File or directory not found: " << path << std::endl;
        continue;
      }

      if (status.type() == llvm::sys::fs::file_type::status_error)
      {
        std::cerr << "Status error on: " << path << std::endl;
        continue;
      }

      SLog(util::STATUS) << "Parsing " << path;
      
      // Open database
      _w = ctx_.workspace;
      
      Context context;      
      util::OdbTransaction trans(*_w->getDb());

      // Step 1: init
      if (!trans([&, this]()
        {
          return initParse(path, context);
        }))
      {
        SLog(util::ERROR) << "Init failed, skip parsing.";
        std::exit(1);
        return false;
      }

      // Step 2: persist parser options and call traversals
      if (!context.partiallyParsed)
      {
        trans([&, this]() {
          for (auto& it : ctx_.props.options)
          {
            model::Option opt;
            opt.key = it.first;
            opt.value = it.second;
            
            _w->getDb()->persist(opt);
          }
        });
      }

      // Count files:
      std::size_t sourceCurrentCount = 0;

      // Statistics:
      std::size_t numOfSuccess = 0;
      std::size_t numOfFails = 0;
      std::size_t numOfActions = 0;
      std::size_t numOfQueuedActions = 0;
        
      // Step 3: parse files
      ActionProcessor actProc(*_w->getDb());
      for (;;)
      {
        std::cout << "PROC actions" << std::endl;
        ParserTask task;

        if (!trans([&, this]()
        {
          if (!getNextTask(context, task))
          {
            return false;
          }
          return true;
        })) break;

        ++numOfActions;
        if (numOfActions % 1000 == 0)
        {
          SLog(util::STATUS) << numOfActions << " actions processed...";
        }

        if (task.action->type == model::BuildAction::Link)
        {
          if (task.action->state != model::BuildAction::StParsed)
          {
            task.action->state = model::BuildAction::StParsed;
            trans([&, this]() {
              _w->getDb()->update(task.action);
            });
          }

          ++numOfSuccess;
          continue;
        }

        AsyncBuildAction asAct;
        if (task.fullCommandLine)
        {
          asAct = parseCommand(task, numOfFails);
        }
        else
        {
          asAct = parseFiles(task, numOfFails);
        }

        if (!asAct.parseResults.empty())
        {
          ++numOfQueuedActions;
          actProc.queue(std::move(asAct));
        }

      }
    }
  }
    return true;
}

bool GeneralParser::iterateDirectoryRecursive(
  TraverseContext& context_,
  const std::string& path_,
  std::vector<Traversal::DirIterCallback> callbacks_)
{
  std::error_code   errCode;
  std::string       fullPath;

  // Convert path to absolute
  {
    llvm::SmallString<2048> path(path_);
    errCode = llvm::sys::fs::make_absolute(path);
    if (errCode)
    {
      SLog(cc::util::ERROR)
          << "Getting absolute failed for "
          << path_;

      // Just skip this one
      return true;
    }

    fullPath = path.str();
  }

  // Status reporting:
  auto currTime = std::chrono::system_clock::now();
  if ((currTime - context_.lastReportTime) >= std::chrono::seconds(15))
  {
    // It's time to report
    SLog(util::STATUS)
      << "Recursive directory iteration: visited "
      << context_.numFilesVisited << " files in "
      << context_.numDirsVisited << " directories so far.";
    context_.lastReportTime = currTime;
  }

  llvm::sys::fs::directory_entry  dirEnt(fullPath);
  llvm::sys::fs::file_status      status;

  errCode = dirEnt.status(status);
  if (errCode)
  {
    SLog(cc::util::ERROR)
        << "Getting status failed for " << fullPath;

    // Just skip this one
    return true;
  }

  cc::parser::Traversal::FileType fileType = cc::parser::Traversal::FileType::Other;
  switch (status.type())
  {
    case llvm::sys::fs::file_type::regular_file:
      ++context_.numFilesVisited;
      fileType = cc::parser::Traversal::FileType::RegularFile;
      break;
    case llvm::sys::fs::file_type::directory_file:
      ++context_.numDirsVisited;
      fileType = cc::parser::Traversal::FileType::Directory;
      break;
    default:
      fileType = cc::parser::Traversal::FileType::Other;
      break;
  }

  // Call callback
  std::vector<Traversal::DirIterCallback>::iterator cbIt = callbacks_.begin();
  while (cbIt != callbacks_.end())
  {
    bool deleteCb = true;
    // std::cout << "fullPath: " << fullPath << std::endl;
    try
    {
      deleteCb = !((*cbIt)(fullPath, fileType));
    }
    catch (const std::exception& ex)
    {
      SLog(cc::util::ERROR)
        << "Got an std::exception from a traversal callback:" << ex.what();
    }
    catch (...)
    {
      SLog(cc::util::ERROR)
        << "Got an unknown exception from a traversal callback!";
    }

    if (deleteCb)
    {
      cbIt = callbacks_.erase(cbIt);
    }
    else
    {
      ++cbIt;
    }
  }

  if (callbacks_.empty())
  {
    return false;
  }

  // Iterate over directory content
  if (fileType == cc::parser::Traversal::FileType::Directory)
  {
    llvm::sys::fs::directory_iterator iter(llvm::Twine(fullPath), errCode);
    llvm::sys::fs::directory_iterator iter_end;

    while (iter != iter_end && !errCode)
    {
      std::string entryPath = iter->path();

      if (cc::util::getFilename(entryPath)[0] != '.' &&
          !iterateDirectoryRecursive(context_, entryPath, callbacks_))
      {
        // Stop iteration
        return false;
      }

      iter.increment(errCode);
    }
  }

  return true;
}

model::BuildActionPtr GeneralParser::addBuildAction(
  model::BuildAction::Type type_,
  uint64_t id,
  const std::string& label_)
{
  model::BuildActionPtr action(new model::BuildAction());

  //action->project = _props.project;
  action->id = id;
  action->label = label_;
  action->type = type_;

  _w->getDb()->persist(action);

  return action;
}


model::BuildParameterPtr GeneralParser::addBuildParameter(
  model::BuildActionPtr action_,
  const std::string& param_)
{
  model::BuildParameterPtr param(new model::BuildParameter());

  param->action = action_;
  param->param = param_;

  _w->getDb()->persist(param);

  return param;
}

model::BuildSourcePtr GeneralParser::addBuildSource(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file;
  /*if (_srcMgr.getCreateFile(path_, file))
  {
    return addBuildSource(action_, file);
  }*/

  SLog(util::ERROR) << "Failed to create build source object for " << path_;
  return model::BuildSourcePtr();
}


model::BuildSourcePtr GeneralParser::addBuildSource(
  model::BuildActionPtr action_,
  model::FilePtr file_)
{
  if (!file_.get())
  {
    SLog(util::CRITICAL) << "Failed to create build source object a null file!";
    return model::BuildSourcePtr();
  }

  model::BuildSourcePtr source(new model::BuildSource());
  source->file = file_;
  source->action = action_;

  _w->getDb()->persist(source);
  return source;
}

model::BuildTargetPtr GeneralParser::addBuildTarget(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file;
  /*if (_srcMgr.getCreateFile(path_, file, SourceManager::NoContent))
  {
    return addBuildTarget(action_, file);
  }*/

  SLog(util::ERROR) << "Failed to create build target object for " << path_;
  return model::BuildTargetPtr();
}

model::BuildTargetPtr GeneralParser::addBuildTarget(
  model::BuildActionPtr action_,
  model::FilePtr file_)
{
  if (!file_.get())
  {
    SLog(util::CRITICAL) << "Failed to create build target object a null file!";
    return model::BuildTargetPtr();
  }

  model::BuildTargetPtr target(new model::BuildTarget());
  target->file = file_;
  target->action = action_;

  _w->getDb()->persist(target);
  return target;
}

void GeneralParser::createNewTask(
  const BuildActions::value_type& jsonAction_,
  ParserTask& task_)
{
  assert(
    !jsonAction_.second.sourceToTarget.empty() &&
    "Empty source to target map!");

  model::BuildAction::Type buildType = model::BuildAction::Other;
  {
    std::string ext = util::getExtension(
      jsonAction_.second.sourceToTarget.begin()->first);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "o" || ext == "so" || ext == "a")
    {
      buildType = model::BuildAction::Link;
    }
    else
    {
      buildType = model::BuildAction::Compile;
    }
  }

  task_.action = addBuildAction(buildType, jsonAction_.first, "");
  task_.fullCommandLine = true;
  task_.options = jsonAction_.second.arguments;

  std::map<std::string, model::BuildTargetPtr> targets;
  for (const auto& srcTarget : jsonAction_.second.sourceToTarget)
  {
    model::BuildTargetPtr target;
    auto it = targets.find(srcTarget.second);
    if (it != targets.end())
    {
      target = it->second;
    }
    else
    {
      target = addBuildTarget(task_.action, srcTarget.second);
      targets[srcTarget.second] = target;
    }

    auto source = addBuildSource(task_.action, srcTarget.first);
    task_.files.push_back(std::make_tuple(source, target));
  }
}

AsyncBuildAction GeneralParser::parseFiles(
  const ParserTask& task_,
  std::size_t& numOfFails_)
{
  AsyncBuildAction asAct;
  util::OdbTransaction trans(*_w->getDb());

  asAct.action = task_.action;
//  for (auto& srcTarget : task_.files)
//  {
//    model::BuildSourcePtr sourcePtr = std::get<0>(srcTarget);
//    if (!sourcePtr)
//    {
//      SLog(util::CRITICAL) << "NULL source file!";
//
//      if (task_.action->state != model::BuildAction::StSkipped)
//      {
//        task_.action->state = model::BuildAction::StSkipped;
//        trans([&, this]() {
//          _w->getDb()->update(task_.action);
//        });
//      }
//
//      ++numOfFails_;
//      continue;
//    }
//
//    std::string sourcePath(sourcePtr->file->path);
//    std::string targetPath;
//
//    model::BuildTargetPtr targetPtr = std::get<1>(srcTarget);
//    if (!targetPtr)
//    {
//      // In GeneralProjectParser it's maybe OK
//      SLog(util::WARNING) << "NULL target file!";
//    }
//    else
//    {
//      targetPath = targetPtr->file->path;
//    }
//
//    std::future<ParseResult> parserResult;
//    std::shared_ptr<IFileParser> parser = FileParser::instance()
//      .getParser(sourcePath);
//    if (!parser)
//    {
//      // It could be a linker command so it not necessarily bad thing
//      SLog(util::WARNING) << "Parser not found for " << sourcePath << "!";
//
//      std::promise<ParseResult> prom;
//      prom.set_value(PARSE_SUCCESS);
//      parserResult = prom.get_future();
//    }
//    else
//    {
//      parserResult =  trans([&, this]() {
//        return parser->parse(_props, task_.action, _srcMgr, sourcePath,
//          targetPath, task_.options);
//      });
//    }
//
//    asAct.parseResults.emplace_back(AsyncBuildResult {
//      std::move(parserResult),
//      sourcePath
//    });
//  }

  return asAct;
}

AsyncBuildAction GeneralParser::parseCommand( const ParserTask& task_,
  std::size_t& numOfFails_)
{
  AsyncBuildAction asAct;
  util::OdbTransaction trans(*_w->getDb());
  if (task_.files.empty())
  {
    return asAct;
  }

//  asAct.action = task_.action;
//  auto sourcePath = std::get<0>(task_.files[0])->file->path;
//  auto parser = FileParser::instance().getParser(sourcePath);
//
//  std::future<ParseResult> parserResult;
//  if (!parser)
//  {
//    // It could be a linker command so it not necessarily bad thing
//    SLog(util::WARNING) << "Parser not found for " << sourcePath << "!";
//
//    std::promise<ParseResult> prom;
//    prom.set_value(PARSE_SUCCESS);
//    parserResult = prom.get_future();
//  }
//  else
//  {
//    IFileParser::BuildSourceTargets sourceToTarget;
//    for (const auto& srcTarget : task_.files)
//    {
//      sourceToTarget.push_back(std::make_pair(
//       std::get<0>(srcTarget)->file,
//       std::get<1>(srcTarget)->file));
//    }
//
//    parserResult =  trans([&, this]() {
//      return parser->parse(_props, task_.action, _srcMgr, task_.options,
//        sourceToTarget);
//    });
//  }
//
//  std::string displaySourcePath = sourcePath;
//  for (std::size_t i = 1; i < task_.files.size(); ++i)
//  {
//    displaySourcePath += ", " + std::get<0>(task_.files[i])->file->path;
//  }
//
//  asAct.parseResults.emplace_back(AsyncBuildResult {
//    std::move(parserResult),
//    displaySourcePath
//  });

  return asAct;
}

void GeneralParser::loadTask(
  model::BuildActionPtr action_,
  const BuildActions::value_type& jsonAction_,
  ParserTask& task_)
{
  task_.action = action_;
  task_.options = jsonAction_.second.arguments;
  task_.fullCommandLine = true;

  // Targets are not used by ProjectParser if fullCommandLine is true, so we
  // won't pair sources to targets
  bool oneToOne = action_->sources.size() == action_->targets.size();
  for (std::size_t i = 0; i < action_->sources.size(); ++i)
  {
    model::BuildTargetPtr target;
    model::BuildSourcePtr source;

    source = action_->sources[i].load();
    source->file.load();
    if (oneToOne)
    {
      target = action_->targets[i].load();
      target->file.load();
    }
    else if (!action_->targets.empty())
    {
      target = action_->targets[0].load();
      target->file.load();
    }
    // else keep target null

    task_.files.push_back(std::make_tuple(source, target));
  }
}

bool GeneralParser::getNextTask(
  Context& context_,
  ParserTask& task_)
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
extern "C"
{
    std::unique_ptr<GeneralParser> make()
    {
        return std::unique_ptr<GeneralParser>(new GeneralParser);
    }
}

}
}
