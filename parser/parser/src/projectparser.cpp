#include <algorithm>
#include <vector>
#include <mutex>
#include <tuple>
#include <atomic>
#include <chrono>
#include <model/workspace.h>

#include <parser/projectparser.h>
#include <parser/crashprotect.h>

#include <model/project.h>
#include <model/project-odb.hxx>
#include <model/buildaction-odb.hxx>
#include <model/buildparameter-odb.hxx>
#include <model/buildsource-odb.hxx>
#include <model/buildtarget-odb.hxx>
#include <model/option-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/statistics.h>
#include <model/statistics-odb.hxx>

#include <util/streamlog.h>
#include <util/util.h>
#include <util/odbtransaction.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/Twine.h>

#include "fileparser/fileparser.h"

#include <indexer-api/indexerprocess.h>

#include "actionprocessor.h"

namespace
{

using cc::parser::ProjectParser;

/**
 * Iterates over the given traversals and calls the given function object for
 * each traversal. If a call throws an error, it removes the traversal from the
 * traversal vector.
 *
 * @param traversals_ vector of traversals.
 * @param func_ a function to call.
 */
void callTraversalsSafe(
  ProjectParser::Traversals& traversals_,
  std::function<void(cc::parser::Traversal&)> func_)
{
  auto iter = traversals_.begin();
  while (iter != traversals_.end())
  {
    try
    {
      func_(**iter);
      ++iter;
    }
    catch (std::exception& ex_)
    {
      SLog(cc::util::ERROR)
        << "Traversal thrown an exception: "
        << ex_.what()
        << "  Skipping this traversal.";

      iter = traversals_.erase(iter);
    }
    catch (...)
    {
      SLog(cc::util::ERROR)
        << "Traversal thrown an unknown exception! Skipping this traversal.";

      iter = traversals_.erase(iter);
    }
  }
}

} // anonymous

namespace cc
{
namespace parser
{

ProjectParser::Traversals ProjectParser::_traversals;

ProjectParser::ProjectParser(
  std::shared_ptr<model::Workspace> w_,
  ParseProps& props_,
  SourceManager& srcMgr_) :
  _w(w_),
  _props(props_),
  _srcMgr(srcMgr_)
{
  crash::initHandler();
}

model::BuildActionPtr ProjectParser::addBuildAction(
  model::BuildAction::Type type_,
  uint64_t id,
  const std::string& label_)
{
  model::BuildActionPtr action(new model::BuildAction());

  action->project = _props.project;
  action->id = id;
  action->label = label_;
  action->type = type_;

  _w->getDb()->persist(action);

  return action;
}

model::BuildParameterPtr ProjectParser::addBuildParameter(
  model::BuildActionPtr action_,
  const std::string& param_)
{
  model::BuildParameterPtr param(new model::BuildParameter());

  param->action = action_;
  param->param = param_;

  _w->getDb()->persist(param);

  return param;
}

model::BuildSourcePtr ProjectParser::addBuildSource(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file;
  if (_srcMgr.getCreateFile(path_, file))
  {
    return addBuildSource(action_, file);
  }

  SLog(util::ERROR) << "Failed to create build source object for " << path_;
  return model::BuildSourcePtr();
}


model::BuildSourcePtr ProjectParser::addBuildSource(
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

model::BuildTargetPtr ProjectParser::addBuildTarget(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file;
  if (_srcMgr.getCreateFile(path_, file, SourceManager::NoContent))
  {
    return addBuildTarget(action_, file);
  }

  SLog(util::ERROR) << "Failed to create build target object for " << path_;
  return model::BuildTargetPtr();
}

model::BuildTargetPtr ProjectParser::addBuildTarget(
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

void ProjectParser::parse(const std::string& path_, ParserCallback callback_)
{
  SLog(util::STATUS) << "Parsing " << path_;

  ParserContext context;

  util::OdbTransaction trans(*_w->getDb());

  // Step 1: init
  if (!trans([&, this]()
    {
      return initParse(path_, context);
    }))
  {
    SLog(util::ERROR) << "Init failed, skip parsing.";
    std::exit(1);
    return;
  }

  // Step 2: persist parser options and call traversals
  if (!context.partiallyParsed)
  {
    trans([&, this]() {
      for (auto& it : _props.options)
      {
        model::Option opt;
        opt.key = it.first;
        opt.value = it.second;

        _w->getDb()->persist(opt);
      }
    });
  }

  if (!context.partiallyParsed)
  {
    traverseRoots(context.roots, _props.options, _srcMgr);
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

    callback_(
      static_cast<int>(++sourceCurrentCount),
      static_cast<int>(context.sourceFileCount));
  }

  SLog(util::STATUS) << numOfQueuedActions << " actions queued total.";

  actProc.setTotalActionCount(numOfActions);
  actProc.stopAndWait();

  // Step 4: post-parse
  {
    std::unique_ptr<search::IndexerServiceIf> indexer;

    auto indexDirIt = _props.options.find("searchIndexDir");
    if (indexDirIt != _props.options.end())
    {
      indexer.reset(new IndexerProcess(indexDirIt->second,
        IndexerProcess::OpenMode::Merge));
    }
    else
    {
      indexer.reset(new search::IndexerServiceNull());
    }

    for (auto& fileparser : FileParser::instance().getParsers())
    {
      std::size_t succ = 0;
      std::size_t fail = 0;

      fileparser->postParse(_props, succ, fail, *indexer);

      numOfSuccess += succ;
      numOfFails += fail;
    }

    indexer->buildSuggestions();

    util::OdbTransaction transaction(
      std::shared_ptr<odb::database>{_w->getDb(), util::NoDelete()});
    transaction([&, this]() {
      addFileStatistics();
    });
  }

  SLog(util::STATUS)
    << "Parsing is done." << std::endl
    << "Successfully parsed " << numOfSuccess << " files." << std::endl
    << "Partially parsed " << numOfFails << " files.";
}

AsyncBuildAction ProjectParser::parseFiles(
  const ParserTask& task_,
  std::size_t& numOfFails_)
{
  AsyncBuildAction asAct;
  util::OdbTransaction trans(*_w->getDb());

  asAct.action = task_.action;
  for (auto& srcTarget : task_.files)
  {
    model::BuildSourcePtr sourcePtr = std::get<0>(srcTarget);
    if (!sourcePtr)
    {
      SLog(util::CRITICAL) << "NULL source file!";

      if (task_.action->state != model::BuildAction::StSkipped)
      {
        task_.action->state = model::BuildAction::StSkipped;
        trans([&, this]() {
          _w->getDb()->update(task_.action);
        });
      }

      ++numOfFails_;
      continue;
    }

    std::string sourcePath(sourcePtr->file->path);
    std::string targetPath;

    model::BuildTargetPtr targetPtr = std::get<1>(srcTarget);
    if (!targetPtr)
    {
      // In GeneralProjectParser it's maybe OK
      SLog(util::WARNING) << "NULL target file!";
    }
    else
    {
      targetPath = targetPtr->file->path;
    }

    std::future<ParseResult> parserResult;
    std::shared_ptr<IFileParser> parser = FileParser::instance()
      .getParser(sourcePath);
    if (!parser)
    {
      // It could be a linker command so it not necessarily bad thing
      SLog(util::WARNING) << "Parser not found for " << sourcePath << "!";

      std::promise<ParseResult> prom;
      prom.set_value(PARSE_SUCCESS);
      parserResult = prom.get_future();
    }
    else
    {
      parserResult =  trans([&, this]() {
        return parser->parse(_props, task_.action, _srcMgr, sourcePath,
          targetPath, task_.options);
      });
    }

    asAct.parseResults.emplace_back(AsyncBuildResult {
      std::move(parserResult),
      sourcePath
    });
  }

  return asAct;
}


AsyncBuildAction ProjectParser::parseCommand(
  const ParserTask& task_,
  std::size_t& numOfFails_)
{
  AsyncBuildAction asAct;
  util::OdbTransaction trans(*_w->getDb());
  if (task_.files.empty())
  {
    return asAct;
  }

  asAct.action = task_.action;
  auto sourcePath = std::get<0>(task_.files[0])->file->path;
  auto parser = FileParser::instance().getParser(sourcePath);

  std::future<ParseResult> parserResult;
  if (!parser)
  {
    // It could be a linker command so it not necessarily bad thing
    SLog(util::WARNING) << "Parser not found for " << sourcePath << "!";

    std::promise<ParseResult> prom;
    prom.set_value(PARSE_SUCCESS);
    parserResult = prom.get_future();
  }
  else
  {
    IFileParser::BuildSourceTargets sourceToTarget;
    for (const auto& srcTarget : task_.files)
    {
      sourceToTarget.push_back(std::make_pair(
       std::get<0>(srcTarget)->file,
       std::get<1>(srcTarget)->file));
    }

    parserResult =  trans([&, this]() {
      return parser->parse(_props, task_.action, _srcMgr, task_.options,
        sourceToTarget);
    });
  }

  std::string displaySourcePath = sourcePath;
  for (std::size_t i = 1; i < task_.files.size(); ++i)
  {
    displaySourcePath += ", " + std::get<0>(task_.files[i])->file->path;
  }

  asAct.parseResults.emplace_back(AsyncBuildResult {
    std::move(parserResult),
    displaySourcePath
  });

  return asAct;
}

void ProjectParser::traverseRoots(
  const std::map<std::string, std::string>& projectRoots_,
  const std::map<std::string, std::string>& projectOptions_,
  SourceManager& srcMgr_)
{
  std::vector<std::string> roots;

  for (const auto& currentRoot : projectRoots_)
  {
    // this algorithm is simple and slow but we have only a few project roots,
    // so its not a problem.

    if (currentRoot.first == "root")
    {
      // This is the file system root (/), skip it.
      continue;
    }

    // Find nested roots.
    bool addCurrent = true;
    std::vector<std::string>::iterator it = roots.begin();
    while (it != roots.end())
    {
      if (currentRoot.second.find(*it) == 0)
      {
        // A nested path in the list.
        it = roots.erase(it);
      }
      else if (it->find(currentRoot.second) == 0)
      {
        // this is a nested root
        addCurrent = false;
        break;
      }
      else
      {
        ++it;
      }
    }

    if (addCurrent)
    {
      // std::cout << "Added a root: " << currentRoot.second << std::endl;
      roots.push_back(currentRoot.second);
    }
  }

  // Step #1: Call beforeTraverse callback.
  callTraversalsSafe(_traversals,[&projectOptions_, &srcMgr_](Traversal& trav_){
    trav_.beforeTraverse(projectOptions_, srcMgr_);
  });

  // Step #2: Call traverse and endTraverse on all root.
  for (const std::string& root : roots)
  {
    std::vector<Traversal::DirIterCallback> callbacks;

    // Step #2a: Call traverse callback.
    callTraversalsSafe(_traversals,
      [&callbacks, &root, &srcMgr_] (Traversal& trav_) {
        Traversal::DirIterCallback callback = trav_.traverse(root, srcMgr_);
        if (callback)
        {
          callbacks.push_back(callback);
        }
      });

    {
      util::OdbTransaction trans(*_w->getDb());
      trans([&, this] {
        // Step #2b: Call non-empty iter-callback for all files in the current
        // root directory.
        try
        {
          TraverseContext context;
          iterateDirectoryRecursive(context, root, callbacks);
        }
        catch (std::exception& ex_)
        {
          SLog(util::ERROR)
            << "Traverse roots (recursive) failed with exception: "
            << ex_.what();
        }
        catch (...)
        {
          SLog(util::ERROR)
            << "Traverse roots (recursive) failed with unknown exception!";
        }

        // Step #2c: Call endTraverse for all non-empty iter-callback returning
        // traversal.
        callTraversalsSafe(_traversals, [&root, &srcMgr_](Traversal& trav_) {
          trav_.endTraverse(root, srcMgr_);
        });
      });
    }
  }

  // Step #3: Call afterTraverse callbacks.
  callTraversalsSafe(_traversals, [&srcMgr_](Traversal& trav_) {
    trav_.afterTraverse(srcMgr_);
  });
}

bool ProjectParser::iterateDirectoryRecursive(
  ProjectParser::TraverseContext& context_,
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

ParseProps ProjectParser::createParseProperties(
  std::shared_ptr<model::Workspace> workspace_)
{
  ParseProps pp;
  model::transaction tr = workspace_->getTransaction();

  auto projects = workspace_->getDb()->query<model::Project>();
  if (!projects.empty())
  {
    pp.project.reset(new model::Project(*projects.begin()));
    tr->commit();
    return pp;
  }

  model::ProjectPtr project(new model::Project());

  workspace_->getDb()->persist(project);

  pp.project = project;

  tr->commit();

  return pp;
}

void ProjectParser::registerTraversal(std::shared_ptr<Traversal> traversal_)
{
  _traversals.push_back(traversal_);
}

void ProjectParser::deregisterTraversal(std::shared_ptr<Traversal> traversal_)
{
  _traversals.erase(std::remove(_traversals.begin(), _traversals.end(),
    traversal_), _traversals.end());
}

void ProjectParser::addFileStatistics()
{
  using namespace model;

  auto getLang = [](const File& file) -> std::string
    {
        switch(file.type)
        {
          case File::CSource:
          case File::CxxSource:    return "C++";
          case File::JavaSource:   return "Java";
          case File::ErlangSource: return "Erlang";
          case File::BashScript:   return "Bash";
          case File::PerlScript:   return "Perl";
          case File::PythonScript: return "Python";
          case File::RubyScript:   return "Ruby";
          case File::SqlScript:    return "SQL";
          case File::JavaScript:   return "JavaScript";

          case File::GenericFile:  return "Other";

          default: break;
        }

        return "";
    };

  auto getParseStatus = [](const File& file) -> std::string
    {
        switch(file.parseStatus)
        {
          case File::PSPartiallyParsed:   return "Partially Parsed files";
          case File::PSFullyParsed:       return "Fully Parsed files";

          default: break;
        }

        return "";
    };

  typedef std::map<std::string, int> ValueMap;

  std::map<std::string, ValueMap> statistics;

  for (auto& file : _w->getDb()->query<model::File>())
  {
    auto lang        = getLang(file);
    auto parseStatus = getParseStatus(file);

    if (!lang.empty() && !parseStatus.empty())
    {
      ++statistics[lang][parseStatus];
    }

    if (!lang.empty() && file.inSearchIndex)
    {
      ++statistics[lang]["In Search Index"];
    }
  }

  for (const auto& langStat : statistics)
  {
    auto& lang = langStat.first;

    for (const auto& stat : langStat.second)
    {
      auto& status = stat.first;
      auto  count  = stat.second;

      _w->addStatistics(lang, status, count);
    }
  }

}

} //parser
} //cc
