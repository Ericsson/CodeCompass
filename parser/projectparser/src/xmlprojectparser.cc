#include <projectparser/xmlprojectparser.h>

#include <fileparser/fileparser.h>
#include <model/workspace.h>
#include <model/option.h>
#include <model/option-odb.hxx>
#include <model/buildaction-odb.hxx>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/odbtransaction.h>
#include <util/filesystem.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/Twine.h>

#include <iostream>
#include <fstream>

namespace cc
{
namespace parser
{

bool XmlProjectParser::accept(const std::string& path_)
{
  llvm::sys::fs::directory_entry de(path_);
  llvm::sys::fs::file_status status;

  de.status(status);

  if (status.type() == llvm::sys::fs::file_type::regular_file &&
      util::getExtension(path_) == "xml")
  {
    return true;
  }

  return false;
}

bool XmlProjectParser::initParse(
  const std::string& path_,
  ProjectParser::ParserContext& context_)
{
  XMLParser               parser;
  XMLParser::BuildActions actions;
  std::map<std::string, std::string> xmlOptions;

  if (!parser.parse(path_, actions, xmlOptions))
  {
    SLog(util::ERROR) << "XML parsing failed!";
    return false;
  }
  
  for (const auto& option : xmlOptions)
  {
    if (_props.options.find(option.first) == _props.options.end() ||
        _props.options[option.first].empty())
    {
      _props.options[option.first] = option.second;
    }
  }

  auto option = _w->getDb()->query<model::Option>(false);
  if (option.begin() == option.end())
  {
    context_.partiallyParsed = false;
  }
  else
  {
    context_.partiallyParsed = true;
  }
  
  context_.roots = actions.roots;
  context_.sourceFileCount = 0;
  for (auto action : actions.actions)
  {
    context_.sourceFileCount += action.sources.size();
  }
  
  _actions = std::move(actions);
  _actionIterator = _actions.actions.begin();

  // We have to load already created build actions (see getNextTask for usage)
  if (context_.partiallyParsed)
  {
    SLog(util::STATUS) << "Filling action state cache...";
    auto res = _w->getDb()->query<model::BuildActionIdState>();
    for (const model::BuildActionIdState& act : res)
    {
      _actionStateInDb[act.id] = act.state;
    }
  }

  return true;
}
  
bool XmlProjectParser::getNextTask(
  ProjectParser::ParserContext& context_,
  ProjectParser::ParserTask& task_)
{
  for (; _actionIterator != _actions.actions.end(); ++_actionIterator)
  {
    XMLParser::BuildAction& action = *_actionIterator;
    
    // Get action if it previously persisted
    model::BuildActionPtr buildAct;
    {
      auto iter = _actionStateInDb.find(action.id);
      if (iter != _actionStateInDb.end())
      {
        model::BuildActionIdState idSt;
        idSt.id = iter->first;
        idSt.state = iter->second;

        // free some memory
        _actionStateInDb.erase(iter);

        if (idSt.state != model::BuildAction::StCreated)
        {
          // Already parsed!
          SLog() << "Action " << action.id << " is already done!";
          continue;
        }

        buildAct = _w->getDb()->load<model::BuildAction>(idSt.id);
      }
    }

    if (_skippableActions.find(action.id) != _skippableActions.end())
    {
      // Set action state to skipped
      if (buildAct && buildAct->state != model::BuildAction::StSkipped)
      {
        buildAct->state = model::BuildAction::StSkipped;
        _w->getDb()->update(*buildAct);
      }

      SLog() << "Skipping action " << action.id << "!";
      continue;
    }

    if (!_debugActions.empty() &&
         _debugActions.find(action.id) == _debugActions.end())
    {
      continue;
    }

    if (buildAct)
    {
      // This is a build action that is created by a previous execution but not
      // finished for some reason. All sources and targets are already persisted
      // we just need to match them.

      task_.action = buildAct;
      task_.options = action.options;

      std::vector<model::BuildTargetPtr> targets;
      for (auto& targetPtr : buildAct->targets)
      {
        model::BuildTargetPtr target = targetPtr.load();

        target->file.load();
        targets.push_back(target);
      }

      for (auto& sourcePtr : buildAct->sources)
      {
        model::BuildSourcePtr source = sourcePtr.load();
        source->file.load();

        model::BuildTargetPtr target = findTarget(source, targets);
        task_.files.emplace_back(std::make_tuple(source, target));
      }
    }
    else
    {
      // This is a new build action

      model::BuildAction::Type buildType = model::BuildAction::Other;
      if (action.type == "compile")
      {
        buildType = model::BuildAction::Compile;
      }
      else if (action.type == "link")
      {
        buildType = model::BuildAction::Link;
      }

      task_.action = addBuildAction(buildType, action.id, action.label);
      task_.options = action.options;

      std::vector<model::BuildTargetPtr> targets;
      for (auto& target : action.targets)
      {
        targets.push_back(addBuildTarget(task_.action, target));
      }

      for (auto& option : action.options)
      {
        addBuildParameter(task_.action, option);
      }

      for (auto& source : action.sources)
      {
        model::BuildSourcePtr modelSource =addBuildSource(task_.action, source);
        if (!modelSource)
        {
          // skip this source
          continue;
        }
        
        model::BuildTargetPtr modelTarget = findTarget(modelSource, targets);
        
        task_.files.emplace_back(std::make_tuple(modelSource, modelTarget));
      }
    }
    
    ++_actionIterator;
    return true;
  }
  
  return false;
}

model::BuildTargetPtr XmlProjectParser::findTarget(
    model::BuildSourcePtr source,
    const std::vector<model::BuildTargetPtr>& targets)
{
  if (targets.size() == 1)
    return targets[0];

  std::string fileName = util::getFilenameWithoutExtension(source->file->path);
  for (auto& target : targets)
  {
    if (fileName == util::getFilenameWithoutExtension(target->file->path))
      return target;
  }

  return {};
}

} // parser
} // cc
