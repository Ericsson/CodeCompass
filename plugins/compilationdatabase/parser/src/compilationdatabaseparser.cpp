#include <memory>
#include <algorithm>
#include <unordered_set>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>

#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <odb/query.hxx>

#include <parser/compilationdatabaseparser.h>
#include <parser/sourcemanager.h>

#include <util/odbtransaction.h>
#include <util/dbutil.h>
#include <util/hash.h>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>

namespace 
{

const std::vector<std::string> cxxSrcExts{
  ".c", ".cc", ".cpp", ".cxx", ".o", ".so", ".a"};

/**
 * This function returns true of the text_ parameter is some kind of source
 * file, i.e. has extension .java or one from cxxSrcExts. If the extension is
 * .java then isJava_ is set to true, otherwise false.
 * TODO: Why isn't .class considered a source file, if .o, .so and .a are so?
 */
bool isSourceFile(const std::string& text_, bool& isJava_)
{
  if (!boost::filesystem::is_regular_file(text_))
    return false;

  std::string ext = boost::filesystem::extension(text_);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".java")
  {
    isJava_ = true;
    return true;
  }

  isJava_ = false;

  return std::find(cxxSrcExts.begin(), cxxSrcExts.end(), ext)
    != cxxSrcExts.end();
}

/**
 * Returns the given file path without the extension.
 */
std::string getPathAndFileWithoutExtension(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return path_.substr(0, path_.size() - ext.size() - 1);
}

/**
 * This function gets the input-output pairs from the compile command.
 *
 * If the compile command contains a .java file then it is considered a Java
 * build action. In this case the inToOut_ will contain the .java file as key
 * and the corresponding .class file as value.
 * If the compile command comtains C/C++ source file(s) and the output is set by
 * -o flag then the output file will be mapped to these source files. Otherwise
 * if no -o flag given but -c is given then the output files will have the same
 * name as the source file but with .o extension.
 * If no -c and -o provided then the output file name will be a.out.
 * If the compile command contains no source files then the function returns
 * false.
 */
bool extractInputOutputs(
  const clang::tooling::CompileCommand& command_,
  std::map<std::string, std::string>& inToOut_)
{
  enum State
  {
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
      sources.insert(arg);
    else if (arg == "-c")
      hasCParam = true;
    else if (arg == "-o")
      state = OParam;
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
      output = getPathAndFileWithoutExtension(*sources.begin()) + ".class";
    }
    else if (hasCParam)
    {
      // It's a C++ project with a -c option.
      for (const auto& src : sources)
        inToOut_[src] = getPathAndFileWithoutExtension(src) + ".o";

      return true;
    }
    else
    {
      // Default for C++
      output = command_.Directory + "/a.out";
    }
  }

  for (const auto& src : sources)
    inToOut_[src] = output;

  return true;
}

/**
 * Generates an ID to the compile command.
 *
 * The generated id is a hash of the command line.
 */
std::uint64_t generateActionId(const clang::tooling::CompileCommand& command_)
{
  std::string data = command_.Directory;
  for (const std::string& cmd : command_.CommandLine)
    data += cmd;

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
  _db = cc::util::createDatabase(ctx_.options["database"].as<std::string>());
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
  namespace fs = boost::filesystem;
  
  fs::path path{path_};
  try
  {
    if(fs::is_regular_file(path))
      return path.extension() == ".json";
  }
  catch (fs::filesystem_error &e)
  {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }

  return false;
}

bool CompilationDatabaseParser::collectBuildActions(
  const std::string& jsonFile_)
{
  std::string errorMsg;

  auto clangDb = clang::tooling::JSONCompilationDatabase::loadFromFile(
    jsonFile_, errorMsg);

  if (!clangDb)
  {
    BOOST_LOG_TRIVIAL(warning)
      << "Failed to parse " << jsonFile_ << " as JSON compilation database!\n"
      << errorMsg;
    return false;
  }

  for (const auto& command : clangDb->getAllCompileCommands())
  {
    std::uint64_t id = generateActionId(command);
    BuildActions::iterator it = _actions.find(id);

    if (it != _actions.end())
      continue;

    BuildCommand buildCommand;
    buildCommand.arguments = command.CommandLine;

    if (!extractInputOutputs(command, buildCommand.sourceToTarget))
    {
      std::string cmdline;
      for (const std::string& arg : buildCommand.arguments)
        cmdline += arg + " ";

      BOOST_LOG_TRIVIAL(warning)
        << "Skipping bad command line in action " << std::to_string(id)
        << ": " << cmdline << "\n"
        << "Maybe there is no input file or it isn't available!";
    }
    else
      _actions[id] = buildCommand;
  }

  return true;
}

void CompilationDatabaseParser::createNewAction(
  const BuildActions::value_type& jsonAction_)
{
  std::uint64_t id = jsonAction_.first;
  const BuildCommand& buildCommand = jsonAction_.second;

  //--- BuildAction ---//

  std::string ext = boost::filesystem::extension(
    buildCommand.sourceToTarget.begin()->first);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  model::BuildActionPtr mBuildAction(new model::BuildAction);

  mBuildAction->id = id;
  mBuildAction->command = boost::algorithm::join(buildCommand.arguments, " ");
  mBuildAction->type
    = ext == ".o" || ext == ".so" || ext == ".a"
    ? model::BuildAction::Link
    : model::BuildAction::Compile;

  _db->persist(mBuildAction);

  //--- BuildSource, BuildTarget ---//

  std::vector<model::BuildSource> sources;
  std::vector<model::BuildTarget> targets;

  for (const auto& srcTarget : buildCommand.sourceToTarget)
  {
    model::BuildSource mBuildSource;
    mBuildSource.file = _ctx.srcMgr.getCreateFile(srcTarget.first);
    mBuildSource.action = mBuildAction;
    sources.push_back(std::move(mBuildSource));

    model::BuildTarget mBuildTarget;
    mBuildTarget.file = _ctx.srcMgr.getCreateFile(srcTarget.second);
    mBuildTarget.action = mBuildAction;
    targets.push_back(std::move(mBuildTarget));
  }

  _ctx.srcMgr.persistFiles();

  for (model::BuildSource buildSource : sources)
    _db->persist(buildSource);
  for (model::BuildTarget buildTarget : targets)
    _db->persist(buildTarget);
}

bool CompilationDatabaseParser::parse()
{        
  for (const std::string& path :
       _ctx.options["input"].as<std::vector<std::string>>())
  {
    BOOST_LOG_TRIVIAL(info)
      << "CompilationDatabaseParser parse path: " << path;

    if (!accept(path))
    {
      BOOST_LOG_TRIVIAL(info) << path << " has not .json extension, skip it!";
      continue;
    }

    if (!collectBuildActions(path))
    {
      BOOST_LOG_TRIVIAL(info)
        << path << " is not a compilation database, skip it!";
      continue;
    }
  }

  util::OdbTransaction trans(_db);

  trans([&, this](){
    typedef odb::result<model::BuildActionId> BAResult;

    BAResult mBuildActionIds = _db->query<model::BuildActionId>();
    std::unordered_set<std::uint64_t> buildActionIds;
    std::transform(
      mBuildActionIds.begin(), mBuildActionIds.end(),
      std::inserter(buildActionIds, buildActionIds.end()),
      [](const model::BuildActionId& buildActionId){
        return buildActionId.id;
      });

    for (const BuildActions::value_type& currentAction : _actions)
      if (buildActionIds.find(currentAction.first) == buildActionIds.end())
        createNewAction(currentAction);
  });

  return true;
}

CompilationDatabaseParser::~CompilationDatabaseParser()
{
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
