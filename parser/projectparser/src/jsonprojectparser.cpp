#include <projectparser/jsonprojectparser.h>

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

#include <clang/Tooling/JSONCompilationDatabase.h>

#include <map>

namespace
{

/**
 * List of file extensions accepted as C++ source file.
 */
const char* const cxxSrcExts[] = {
  "c", "cc", "cpp", "cxx", "o", "so", "a", NULL
};

bool isSourceFile(const std::string& text_, bool& isJava)
{
  llvm::sys::fs::directory_entry de(text_);
  llvm::sys::fs::file_status status;

  de.status(status);
  if (status.type() != llvm::sys::fs::file_type::regular_file)
  {
    return false;
  }

  std::string ext = cc::util::getExtension(text_);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext == "java")
  {
    isJava = true;
    return true;
  }

  for (std::size_t i = 0; cxxSrcExts[i]; ++i)
  {
    if (ext == cxxSrcExts[i])
    {
      return true;
    }
  }

  return false;
}

bool extractInputOutputs(
  const clang::tooling::CompileCommand& command_,
  std::map<std::string, std::string>& inToOut_)
{
  enum State {
    None,
    OParam
  };

  bool isJava = false;
  bool hasCParam = false;
  std::unordered_set<std::string> sources;
  std::string output;

  State state = None;
  for (const std::string& arg : command_.CommandLine)
  {
    if (state == OParam)
    {
      output = arg;
      state = None;
    }
    else if (isSourceFile(arg, isJava))
    {
      sources.insert(arg);
    }
    else if (arg == "-c")
    {
      hasCParam = true;
    }
    else if (arg == "-o")
    {
      state = OParam;
    }
  }

  if (sources.empty())
  {
    // Empty project???
    return false;
  }

  if (output.empty())
  {
    if (isJava)
    {
      // Java, the java parser will fix this if it's wrong
      output = cc::util::getPathAndFileWithoutExtension(*sources.begin())
        + ".class";
    }
    else if (hasCParam)
    {
      // It's a C++ project with a -c option.
      for (const auto& src : sources)
      {
        inToOut_[src] =  cc::util::getPathAndFileWithoutExtension(src) + ".o";
      }

      return true;
    }
    else
    {
      // Default for C++
      output = command_.Directory + "/a.out";
    }
  }

  for (const auto& src : sources)
  {
    inToOut_[src] = output;
  }

  return true;
}

std::uint64_t generateActionId(const clang::tooling::CompileCommand& command_)
{
  std::string data;

  data += command_.Directory;
  for (const std::string& cmd : command_.CommandLine)
  {
    data += cmd;
  }

  return cc::util::fnvHash(data);
}

} // anonymous

namespace cc
{
namespace parser
{

JSonProjectParser::JSonProjectParser(
  std::shared_ptr<model::Workspace> w_,
  ParseProps& props_,
  SourceManager& srcMgr_,
  const ProjectRoots& roots_) :
  ProjectParser(w_, props_, srcMgr_),
  _projRoots(roots_)
{
}

JSonProjectParser::~JSonProjectParser()
{
}

bool JSonProjectParser::accept(const std::string& path_)
{
  llvm::sys::fs::directory_entry de(path_);
  llvm::sys::fs::file_status status;

  de.status(status);

  if (status.type() == llvm::sys::fs::file_type::regular_file &&
      util::getExtension(path_) == "json")
  {
    return true;
  }

  return false;
}

bool JSonProjectParser::initParse(
  const std::string& path_,
  ParserContext& context_)
{
  std::string errorMsg;

  auto clangDb = clang::tooling::JSONCompilationDatabase::loadFromFile(
    path_, errorMsg);
  if (!clangDb)
  {
    SLog(util::ERROR)
      << "Failed to parse " << path_ << " as JSON compilation database!"
      << errorMsg;
    return false;
  }

  auto commands = clangDb->getAllCompileCommands();
  for (const auto& command : commands)
  {
    auto id = generateActionId(command);
    if (_actions.find(id) != _actions.end())
    {
      // Duplicate, drop it
      continue;
    }

    auto& buildCommans = _actions[id];
    buildCommans.arguments = command.CommandLine;

    if (!extractInputOutputs(command, buildCommans.sourceToTarget))
    {
      std::string cmdline;
      for (const std::string& arg : buildCommans.arguments)
      {
        cmdline += arg + " ";
      }

      SLog(util::ERROR)
        << "Skipping bad command line in action " << std::to_string(id)
        << ": " << cmdline << ". Maybe there is no input file or it isn't available!";
      _actions.erase(id);
    }
  }

  auto option = _w->getDb()->query<model::Option>(false);
  context_.partiallyParsed = option.begin() != option.end();
  context_.roots = _projRoots;
  context_.sourceFileCount = clangDb->getAllFiles().size();

  // Add labels to options
  for (auto root : _projRoots)
  {
    if (root.second.empty() || root.second.back() != '/')
    {
      root.second += '/';
    }
    _props.options["paths"] += (_props.options["paths"].empty() ? "" : "|")
                            +   root.first + ':' + root.second;
  }

  _currentAction = _actions.begin();
  return true;
}

bool JSonProjectParser::getNextTask(
  ParserContext& context_,
  ParserTask& task_)
{
  for (; _currentAction != _actions.end(); ++_currentAction)
  {
    model::BuildActionPtr buildAct =
      _w->getDb()->find<model::BuildAction>(_currentAction->first);

    // Skip support
    if (_skippableActions.find(_currentAction->first) != _skippableActions.end())
    {
      // Set action state to skipped
      if (buildAct && buildAct->state != model::BuildAction::StSkipped)
      {
        buildAct->state = model::BuildAction::StSkipped;
        _w->getDb()->update(*buildAct);
      }

      SLog() << "Skipping action " << _currentAction->first << "!";
      continue;
    }

    // Skip already completed actions too
    if (buildAct && buildAct->state == model::BuildAction::StParsed)
    {
      SLog() << "Skipping action " << _currentAction->first << ", it is already completed.";
      continue;
    }

    // Debug support
    if (!_debugActions.empty() &&
         _debugActions.find(_currentAction->first) == _debugActions.end())
    {
      continue;
    }

    if (buildAct)
    {
      loadTask(buildAct, *_currentAction, task_);
    }
    else
    {
      createNewTask(*_currentAction, task_);
    }

    ++_currentAction;
    return true;
  }

  return false;
}

void JSonProjectParser::createNewTask(
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

void JSonProjectParser::loadTask(
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

} // parser
} // cc
