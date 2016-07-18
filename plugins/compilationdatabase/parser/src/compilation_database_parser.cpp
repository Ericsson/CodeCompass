#include <parser/compilation_database_parser.h>

#include <unordered_set>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>

#include <model/buildaction.h>
#include <model/buildsource.h>
#include <model/buildtarget.h>

#include <parser/sourcemanager.h>
#include <util/db/odbtransaction.h>
#include <util/db/dbutil.h>
#include <util/hash.h>

#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/JSONCompilationDatabase.h>


#include <model/buildaction-odb.hxx>



#include <memory>

namespace 
{
  
/**
 * List of file extensions accepted as C++ source file.
 */
const char* const cxxSrcExts[] = {
  ".c", ".cc", ".cpp", ".cxx", ".o", ".so", ".a", NULL
};

bool isSourceFile(const std::string& text_, bool& isJava_)
{
  llvm::sys::fs::directory_entry de(text_);
  llvm::sys::fs::file_status status;

  de.status(status);
  if (status.type() != llvm::sys::fs::file_type::regular_file)
  {
    return false;
  }

  std::string ext = boost::filesystem::extension(text_);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext == ".java")
  {
    isJava_ = true;
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

std::string getPathAndFileWithoutExtension(const std::string path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return path_.substr(0, path_.size() - ext.size() - 1);
}

bool extractInputOutputs(const clang::tooling::CompileCommand& command_,
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
      output = getPathAndFileWithoutExtension(*sources.begin())
        + ".class";
    }
    else if (hasCParam)
    {
      // It's a C++ project with a -c option.
      for (const auto& src : sources)
      {
        inToOut_[src] =  getPathAndFileWithoutExtension(src) + ".o";
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
  std::string data = command_.Directory;
  for (const std::string& cmd : command_.CommandLine)
  {
    data += cmd;
  }

  return cc::util::fnvHash(data);
}

}

namespace cc
{
namespace parser
{

  
CompilationDatabaseParser::CompilationDatabaseParser(ParserContext& ctx_) :
  AbstractParser(ctx_)
{
}
  
std::string CompilationDatabaseParser::getName() const
{
  return "compilationdatabaseparser";
}

std::vector<std::string> CompilationDatabaseParser::getDependentParsers() const
{
  return std::vector<std::string>{};
}

bool CompilationDatabaseParser::accept(const std::string& path_)
{
  namespace fs = ::boost::filesystem;
  
  fs::path path{path_};
  try
  {
    if(fs::is_regular_file(path))
    {
      return path.extension() == ".json";
    }
  }
  catch (fs::filesystem_error &e)
  {
    BOOST_LOG_TRIVIAL(error) <<  e.what();
  }
  return false;
}

model::BuildActionPtr CompilationDatabaseParser::addBuildAction(
  model::BuildAction::Type type_,
  uint64_t id_,
  const std::string& command_)
{
  model::BuildActionPtr action(new model::BuildAction());

  action->id = id_;
  action->command = command_;
  action->type = type_;

  _db->persist(action);
  return action;
}

model::BuildSourcePtr CompilationDatabaseParser::addBuildSource(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file = _ctx.srcMgr.getCreateFile(path_);
  if (file && file.get())
  {
    model::BuildSourcePtr source(new model::BuildSource());
    source->file = file;
    source->action = action_;
    _buildSources.push_back(source);

    return source;
  }

  BOOST_LOG_TRIVIAL(error)
    << "Failed to create build source object for " << path_;
  return model::BuildSourcePtr();
}

model::BuildTargetPtr CompilationDatabaseParser::addBuildTarget(
  model::BuildActionPtr action_,
  const std::string& path_)
{
  model::FilePtr file = _ctx.srcMgr.getCreateFile(path_);
  if (file && file.get())
  {
    model::BuildTargetPtr target(new model::BuildTarget());
    target->file = file;
    target->action = action_;
    _buildTargets.push_back(target);

    return target;
  }

  BOOST_LOG_TRIVIAL(error)
    << "Failed to create build target object for " << path_;
  return model::BuildTargetPtr();
}

bool CompilationDatabaseParser::initParse(const std::string& path_)
{
  std::string errorMsg;

  auto clangDb = clang::tooling::JSONCompilationDatabase::loadFromFile(
    path_, errorMsg);
  if (!clangDb)
  {
    BOOST_LOG_TRIVIAL(error) 
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

    auto& buildCommands = _actions[id];
    buildCommands.arguments = command.CommandLine;

    if (!extractInputOutputs(command, buildCommands.sourceToTarget))
    {
      std::string cmdline;
      for (const std::string& arg : buildCommands.arguments)
      {
        cmdline += arg + " ";
      }

      BOOST_LOG_TRIVIAL(error) 
        << "Skipping bad command line in action " << std::to_string(id)
        << ": " << cmdline << ". Maybe there is no input file or it isn't available!";
      _actions.erase(id);
    }
  }
  
  return true;
}

void CompilationDatabaseParser::createNewAction(
  const BuildActions::value_type& jsonAction_)
{
  assert(!jsonAction_.second.sourceToTarget.empty() &&
    "Empty source to target map!");

  model::BuildAction::Type buildType = model::BuildAction::Other;
  {
    std::string ext = boost::filesystem::extension(
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

  std::string buildCommand = boost::algorithm::join(
      jsonAction_.second.arguments, " ");
  auto action = addBuildAction(buildType, jsonAction_.first, buildCommand);

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
      target = addBuildTarget(action, srcTarget.second);
      targets[srcTarget.second] = target;
    }
    addBuildSource(action, srcTarget.first);
  }
}

bool CompilationDatabaseParser::parse()
{        
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(accept(path))
    {
      BOOST_LOG_TRIVIAL(debug) 
        << "CompilationDatabaseParser parse path: " << path;
      
      _db = cc::util::createDatabase(
        _ctx.options["database"].as<std::string>());
      util::OdbTransaction trans(_db);
      
      //--- Step 1: init ---//
      
      if(!initParse(path))
      {
        BOOST_LOG_TRIVIAL(error) 
        << "Init failed, skip parsing!";
        return false;
      }
      
      //--- Step 3: parse files ---//
      trans([&, this](){
        for (const BuildActions::value_type& currentAction : _actions)
        {
          model::BuildActionPtr buildAct =
            _db->find<model::BuildAction>(currentAction.first);

          if (!buildAct)
          {
            createNewAction(currentAction);
          }
        }
      });
      
      return true;      
    }
  }
  return false;
}

CompilationDatabaseParser::~CompilationDatabaseParser()
{
  _ctx.srcMgr.persistFiles();

  util::OdbTransaction trans(_db);
  trans([&, this](){
    for(const model::BuildSourcePtr& buildSource : _buildSources)
      _db->persist(buildSource);
    for(const model::BuildTargetPtr& buildTarget : _buildTargets)
      _db->persist(buildTarget);
  });
}

extern "C"
{
  std::shared_ptr<CompilationDatabaseParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CompilationDatabaseParser>(ctx_);
  }
}

} // parser
} // cc
