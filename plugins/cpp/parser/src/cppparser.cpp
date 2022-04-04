#include <algorithm>
#include <numeric>
#include <fstream>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/builddirectory.h>
#include <model/builddirectory-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/hash.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <cppparser/cppparser.h>

#include "clangastvisitor.h"
#include "relationcollector.h"
#include "entitycache.h"
#include "ppincludecallback.h"
#include "ppmacrocallback.h"
#include "doccommentcollector.h"
#include "diagnosticmessagehandler.h"

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

class VisitorActionFactory : public clang::tooling::FrontendActionFactory
{
public:
  static void cleanUp()
  {
    MyFrontendAction::_entityCache.clear();
  }

  static void init(ParserContext& ctx_)
  {
    util::OdbTransaction {ctx_.db} ([&] {
      for (const model::CppAstNode& node : ctx_.db->query<model::CppAstNode>())
        MyFrontendAction::_entityCache.insert(node);
    });
  }

  VisitorActionFactory(ParserContext& ctx_) : _ctx(ctx_)
  {
  }

  std::unique_ptr<clang::FrontendAction> create() override
  {
    return std::make_unique<MyFrontendAction>(_ctx);
  }

private:
  class MyConsumer : public clang::ASTConsumer
  {
  public:
    MyConsumer(
      ParserContext& ctx_,
      clang::ASTContext& context_,
      EntityCache& entityCache_)
        : _entityCache(entityCache_), _ctx(ctx_), _context(context_)
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext& context_) override
    {
      {
        ClangASTVisitor clangAstVisitor(
          _ctx, _context, _entityCache, _clangToAstNodeId);
        clangAstVisitor.TraverseDecl(context_.getTranslationUnitDecl());
      }

      {
        RelationCollector relationCollector(
          _ctx, _context);
        relationCollector.TraverseDecl(context_.getTranslationUnitDecl());
      }

      if (!_ctx.options.count("skip-doccomment"))
      {
        DocCommentCollector docCommentCollector(
          _ctx, _context, _entityCache, _clangToAstNodeId);
        docCommentCollector.TraverseDecl(context_.getTranslationUnitDecl());
      }
      else
        LOG(info) << "C++ documentation parser has been skipped.";
    }

  private:
    EntityCache& _entityCache;
    std::unordered_map<const void*, model::CppAstNodeId> _clangToAstNodeId;

    ParserContext& _ctx;
    clang::ASTContext& _context;
  };

  class MyFrontendAction : public clang::ASTFrontendAction
  {
    friend class VisitorActionFactory;

  public:
    MyFrontendAction(ParserContext& ctx_) : _ctx(ctx_)
    {
    }

    virtual bool BeginSourceFileAction(
      clang::CompilerInstance& compiler_) override
    {
      compiler_.createASTContext();
      auto& pp = compiler_.getPreprocessor();

      pp.addPPCallbacks(std::make_unique<PPIncludeCallback>(
        _ctx, compiler_.getASTContext(), _entityCache, pp));
      pp.addPPCallbacks(std::make_unique<PPMacroCallback>(
        _ctx, compiler_.getASTContext(), _entityCache, pp));

      return true;
    }

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler_, llvm::StringRef) override
    {
      return std::unique_ptr<clang::ASTConsumer>(
        new MyConsumer(_ctx, compiler_.getASTContext(), _entityCache));
    }

  private:
    static EntityCache _entityCache;

    ParserContext& _ctx;
  };

  ParserContext& _ctx;
};

EntityCache VisitorActionFactory::MyFrontendAction::_entityCache;

bool CppParser::isSourceFile(const std::string& file_) const
{
  const std::vector<std::string> cppExts{
    ".c", ".cc", ".cpp", ".cxx", ".o", ".so", ".a"};

  std::string ext = fs::extension(file_);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  return std::find(cppExts.begin(), cppExts.end(), ext) != cppExts.end();
}

bool CppParser::isNonSourceFlag(const std::string& arg_) const
{
  return arg_.find("-Wl,") == 0;
}

std::map<std::string, std::string> CppParser::extractInputOutputs(
  const clang::tooling::CompileCommand& command_) const
{
  std::map<std::string, std::string> inToOut;

  enum State
  {
    None,
    OParam
  };

  bool hasCParam = false;
  std::unordered_set<std::string> sources;
  std::string output;

  State state = None;
  for (const std::string& arg : command_.CommandLine)
  {
    if (state == OParam)
    {
      fs::path absolutePath = fs::absolute(arg, command_.Directory);
      output = absolutePath.native();
      state = None;
    }
    else if (isSourceFile(arg) && !isNonSourceFlag(arg))
    {
      fs::path absolutePath = fs::absolute(arg, command_.Directory);
      sources.insert(absolutePath.native());
    }
    else if (arg == "-c")
      hasCParam = true;
    else if (arg == "-o")
      state = OParam;
  }

  if (output.empty() && hasCParam)
  {
    for (const std::string& src : sources)
    {
      std::string extension = fs::extension(src);
      inToOut[src] = src.substr(0, src.size() - extension.size() - 1) + ".o";
    }
  }
  else
  {
    if (output.empty())
      output = command_.Directory + "/a.out";

    for (const std::string& src : sources)
      inToOut[src] = output;
  }

  return inToOut;
}

model::BuildActionPtr CppParser::addBuildAction(
  const clang::tooling::CompileCommand& command_)
{
  util::OdbTransaction transaction(_ctx.db);

  model::BuildActionPtr buildAction(new model::BuildAction);

  std::string extension = fs::extension(command_.Filename);

  buildAction->command = boost::algorithm::join(command_.CommandLine, " ");
  buildAction->type
    = extension == ".o" || extension == ".so" || extension == ".a"
    ? model::BuildAction::Link
    : model::BuildAction::Compile;

  transaction([&, this]{ _ctx.db->persist(buildAction); });

  return buildAction;
}

void CppParser::addCompileCommand(
  const clang::tooling::CompileCommand& command_,
  model::BuildActionPtr buildAction_,
  bool error_)
{
  util::OdbTransaction transaction(_ctx.db);

  std::vector<model::BuildSource> sources;
  std::vector<model::BuildTarget> targets;
  model::BuildDirectory buildDir;

  for (const auto& srcTarget : extractInputOutputs(command_))
  {
    model::BuildSource buildSource;
    buildSource.file = _ctx.srcMgr.getFile(srcTarget.first);
    buildSource.file->parseStatus = error_
      ? model::File::PSPartiallyParsed
      : model::File::PSFullyParsed;
    _ctx.srcMgr.updateFile(*buildSource.file);
    buildSource.action = buildAction_;
    sources.push_back(std::move(buildSource));

    model::BuildTarget buildTarget;
    buildTarget.file = _ctx.srcMgr.getFile(srcTarget.second);
    buildTarget.action = buildAction_;
    if (buildTarget.file->type != model::File::BINARY_TYPE)
    {
      buildTarget.file->type = model::File::BINARY_TYPE;
      _ctx.srcMgr.updateFile(*buildTarget.file);
    }

    targets.push_back(std::move(buildTarget));
  }
  
  buildDir.directory = command_.Directory;
  buildDir.action = buildAction_;

  _ctx.srcMgr.persistFiles();

  transaction([&, this] {
    for (model::BuildSource buildSource : sources)
      _ctx.db->persist(buildSource);
    for (model::BuildTarget buildTarget : targets)
      _ctx.db->persist(buildTarget);
    _ctx.db->persist(buildDir);
  });
}

int CppParser::parseWorker(const clang::tooling::CompileCommand& command_)
{
  //--- Assemble compiler command line ---//

  std::vector<const char*> commandLine;
  commandLine.reserve(command_.CommandLine.size());
  commandLine.push_back("--");
  std::transform(
    command_.CommandLine.begin() + 1, // Skip compiler name
    command_.CommandLine.end(),
    std::back_inserter(commandLine),
    [](const std::string& s){ return s.c_str(); });

  int argc = commandLine.size();

  std::string buildDir = command_.Directory;
  if (!fs::is_directory(buildDir))
  {
    LOG(debug) << "Compilation directory " << buildDir
               << " is missing, using '/' instead.";
    buildDir = "/";
  }

  std::string compilationDbLoadError;
  std::unique_ptr<clang::tooling::FixedCompilationDatabase> compilationDb(
    clang::tooling::FixedCompilationDatabase::loadFromCommandLine(
      argc,
      commandLine.data(),
      compilationDbLoadError,
      buildDir));

  if (!compilationDb)
  {
    LOG(error)
      << "Failed to create compilation database from command-line. "
      << compilationDbLoadError;
    return 1;
  }

  //--- Save build action ---//

  model::BuildActionPtr buildAction = addBuildAction(command_);

  //--- Start the tool ---//

  VisitorActionFactory factory(_ctx);

  // Use a PhysicalFileSystem as it's thread-safe

  clang::tooling::ClangTool tool(*compilationDb, command_.Filename,
    std::make_shared<clang::PCHContainerOperations>(),
    llvm::vfs::createPhysicalFileSystem().release());

  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts
    = new clang::DiagnosticOptions();
  DiagnosticMessageHandler diagMsgHandler(diagOpts.get(), _ctx.srcMgr, _ctx.db);
  tool.setDiagnosticConsumer(&diagMsgHandler);

  int error = tool.run(&factory);

  //--- Save build command ---//

  addCompileCommand(command_, buildAction, error);

  return error;
}

CppParser::CppParser(ParserContext& ctx_) : AbstractParser(ctx_)
{
}

std::vector<std::vector<std::string>> CppParser::createCleanupOrder()
{
  typedef boost::adjacency_list<boost::vecS, boost::vecS,
    boost::bidirectionalS> Graph;
  typedef boost::adjacency_list<>::vertex_descriptor Vertex;
  typedef std::pair<int, int> Edge;

  Graph g;
  std::vector<Edge> edges;
  std::map<std::string, Vertex> fileNameToVertex;
  std::deque<std::pair<std::string, Vertex>> deleteQueue;

  for(const auto& file : _ctx.fileStatus)
  {
    fileNameToVertex[file.first] = boost::add_vertex(g);
  }

  try
  {
    util::OdbTransaction{_ctx.db}([&]
    {
      for (const auto& item : _ctx.fileStatus)
      {
        auto file = _ctx.srcMgr.getFile(item.first);

        auto inclusions = _ctx.db->query<model::CppHeaderInclusion>(
          odb::query<model::CppHeaderInclusion>::included == file->id);

        for (const auto& inclusion : inclusions)
        {
          bool inserted;
          model::FilePtr includer = inclusion.includer.load();
          boost::graph_traits<Graph>::edge_descriptor e;
          boost::tie(e, inserted) = boost::add_edge(
            fileNameToVertex.at(includer->path),
            fileNameToVertex.at(file->path), g);
        }
      }
    });
  }
  catch (odb::database_exception&)
  {
    LOG(fatal) << "[cppparser] Topological ordering failed!";
    return std::vector<std::vector<std::string>>();
  }

  if (fileNameToVertex.empty())
  {
    LOG(info) << "[cppparser] No changed files to create topological order!";
    return std::vector<std::vector<std::string>>();
  }

  std::vector<std::vector<std::string>> order;
  std::size_t index = 0;
  while (!fileNameToVertex.empty())
  {
    order.resize(order.size() + 1);

    for (const auto& item : fileNameToVertex)
    {
      if (boost::in_degree(item.second, g) == 0)
      {
        order[index].push_back(item.first);
      }
    }

    for (const std::string& path : order[index])
    {
      boost::clear_out_edges(fileNameToVertex[path], g);
      fileNameToVertex.erase(path);
    }

    /* Circular dependencies in the parsed code would cause
     * this loop to be infinite. If no files were put in
     * the current cleanup level, there is probably a
     * circular dependency somewhere. The rest of the
     * to-be-cleaned up files can be put in an additional level.
     */
    if (order[index].size() == 0)
    {
      for (const auto& item : fileNameToVertex)
      {
        order[index].push_back(item.first);
      }

      fileNameToVertex.clear();

      LOG(debug) << "[cppparser] Circular dependency detected.";
    }

    ++index;
  }
  LOG(debug) << "[cppparser] Topology has " << index << " levels.";

  return order;
}

void CppParser::markModifiedFiles()
{
  std::vector<model::FilePtr> filePtrs(_ctx.fileStatus.size());

  std::transform(_ctx.fileStatus.begin(),
                 _ctx.fileStatus.end(),
                 filePtrs.begin(),
                 [this](const auto& item)
                 {
                   if (item.second == IncrementalStatus::MODIFIED ||
                       item.second == IncrementalStatus::DELETED)
                   {
                     return _ctx.srcMgr.getFile(item.first);
                   }
                   else
                   {
                     return std::make_shared<model::File>();
                   }
                 });

  // Detect changed files through C++ header inclusions.
  util::OdbTransaction {_ctx.db} ([&]
  {
    for (const model::FilePtr file : filePtrs)
    {
      if(file)
      {
        markByInclusion(file);
      }
    }
  }); // end of transaction

  // Detect changed translation units through the build actions.
  for (const std::string& input
    : _ctx.options["input"].as<std::vector<std::string>>())
    if (fs::is_regular_file(input))
    {
      std::string errorMsg;

      std::unique_ptr<clang::tooling::JSONCompilationDatabase> compDb
        = clang::tooling::JSONCompilationDatabase::loadFromFile(
          input, errorMsg,
          clang::tooling::JSONCommandLineSyntax::Gnu);

      if (!errorMsg.empty())
      {
        LOG(error) << errorMsg;
        continue;
      }

      // Read the compilation commands from the JSON file
      std::vector<clang::tooling::CompileCommand> compileCommands =
        compDb->getAllCompileCommands();
      std::unordered_set<std::string> commandTexts;
      for (const auto& command : compileCommands)
      {
        commandTexts.insert(
          boost::algorithm::join(command.CommandLine, " "));
      }

      // Load the compilation commands from the workspace database
      util::OdbTransaction {_ctx.db} ([&] {
        for (const model::BuildAction& ba : _ctx.db->query<model::BuildAction>())
        {
          // If a compilation command is found in the workspace database,
          // but not in the JSON file, mark the source files for cleanup.
          if (commandTexts.find(ba.command) == commandTexts.end())
          {
            for(auto buildSourceLazyPtr : ba.sources)
            {
              auto buildSourcePtr = buildSourceLazyPtr.load();
              if (!_ctx.fileStatus.count(buildSourcePtr->file->path))
              {
                _ctx.fileStatus.emplace(buildSourcePtr->file->path, IncrementalStatus::ACTION_CHANGED);
                LOG(debug) << "[cppparser] Build action for file changed: " << buildSourcePtr->file->path;
              }
            }
          }
        }
      }); // end of transaction
    }
}

bool CppParser::cleanupDatabase()
{
  // Construct the topological order of the files.
  // Each subvector is layer of leaves.

  std::vector<std::vector<std::string>> topologicallyOrderedFiles =
    createCleanupOrder();

  // Calculate the complete number of cleanup jobs.

  int threadNum = _ctx.options["jobs"].as<int>();
  int numCleanupJobs = std::accumulate(
    topologicallyOrderedFiles.begin(),
    topologicallyOrderedFiles.end(),
    0,
    [](int sum, const auto& level)
    {
      return sum + level.size();
    }
  );
  bool allJobsSucceded = true;

  // Define the cleanup action for a single file.

  auto cleanupCommand = [this, &numCleanupJobs, &allJobsSucceded](CleanupJob& job_)
  {

    LOG(info)
      << "[cppparser] "
      << '(' << job_.index << '/' << numCleanupJobs << ')'
      << " Database cleanup: " << job_.path;

    bool success = this->cleanupWorker(job_.path);

    if (!success)
    {
      allJobsSucceded = false;
      LOG(error)
        << "[cppparser] "
        << '(' << job_.index << '/' << numCleanupJobs << ')'
        << " Database cleanup for " << job_.path << " has been failed.";
    }
    else
      LOG(debug)
        << "[cppparser] "
        << '(' << job_.index << '/' << numCleanupJobs << ')'
        << " Database cleanup for " << job_.path << " has succeeded.";
  };

  // Process all the layers of the graph.
  // The elements of a single layer can be cleaned up parallely.

  int levelIndex = 0;
  std::size_t jobIndex = 0;
  for (const auto& level : topologicallyOrderedFiles)
  {
    std::unique_ptr<util::JobQueueThreadPool<CleanupJob>> pool =
      util::make_thread_pool<CleanupJob>(threadNum, cleanupCommand);

    LOG(debug) << "[cppparser] Started cleanup level: " << ++levelIndex;
    for (const std::string& filePath : level)
    {
      CleanupJob job(filePath, ++jobIndex);
      pool->enqueue(job);
    }

    pool->wait();
    LOG(debug)
      << "[cppparser] Finished cleanup level: " << levelIndex
      << " (" << jobIndex << " jobs)";
  }

  return allJobsSucceded;
}

bool CppParser::cleanupWorker(const std::string& path_)
{
  const unsigned short maxTries = 3;
  for (unsigned short tryCount = 1; ; ++tryCount)
  {
    try
    {
      util::OdbTransaction{_ctx.db}([&]
      {
        switch (_ctx.fileStatus[path_])
        {
          case IncrementalStatus::MODIFIED:
          case IncrementalStatus::DELETED:
          case IncrementalStatus::ACTION_CHANGED:
          {
            // Fetch file from SourceManager by path
            model::FilePtr delFile = _ctx.srcMgr.getFile(path_);

            // Query CppAstNode
            auto defCppAstNodes = _ctx.db->query<model::CppAstNode>(
              odb::query<model::CppAstNode>::location.file == delFile->id);

            for (const model::CppAstNode& astNode : defCppAstNodes)
            {
              // Delete CppEntity
              _ctx.db->erase_query<model::CppEntity>(odb::query<model::CppEntity>::astNodeId == astNode.id);

              if (astNode.astType == model::CppAstNode::AstType::Definition)
              {
                // Delete CppInheritance
                _ctx.db->erase_query<model::CppInheritance>(
                  odb::query<model::CppInheritance>::derived == astNode.entityHash);

                // Delete CppFriendship
                _ctx.db->erase_query<model::CppFriendship>(
                  odb::query<model::CppFriendship>::target == astNode.entityHash);
              }
            }

            // Delete BuildAction
            auto delSources = _ctx.db->query<model::BuildSource>(
              odb::query<model::BuildSource>::file == delFile->id);
            for (const model::BuildSource& source : delSources)
            {
              _ctx.db->erase<model::BuildAction>(source.action->id);
            }

            // Delete CppEdge (connected to File)
            _ctx.db->erase_query<model::CppEdge>(odb::query<model::CppEdge>::from == delFile->id);

            break;
          }

          case IncrementalStatus::ADDED:
            // Empty deliberately
            break;
        }
      });
    }
    catch (odb::deadlock& ex)
    {
      if (tryCount < maxTries)
        LOG(warning) << "[cppparser] Transaction deadlock occurred, "
          << "retrying (" << (tryCount + 1) << '/' << maxTries << "): " << path_;
      else
      {
        LOG(error) << "[cppparser] Transaction deadlock occurred, aborting: " << path_;
        return false;
      }
    }
    catch (odb::database_exception&)
    {
      LOG(fatal) << "[cppparser] Transaction failed!";
      return false;
    }

    return true;
  }
}

bool CppParser::parse()
{
  initBuildActions();
  VisitorActionFactory::init(_ctx);

  bool success = true;

  for (const std::string& input
    : _ctx.options["input"].as<std::vector<std::string>>())
    if (fs::is_regular_file(input))
      success
        = success && parseByJson(input, _ctx.options["jobs"].as<int>());

  VisitorActionFactory::cleanUp();
  _parsedCommandHashes.clear();

  return success;
}

void CppParser::initBuildActions()
{
  util::OdbTransaction {_ctx.db} ([&] {
    for (const model::BuildAction& ba : _ctx.db->query<model::BuildAction>())
      _parsedCommandHashes.insert(util::fnvHash(ba.command));
  });
}

void CppParser::markByInclusion(model::FilePtr file_)
{
  auto inclusions = _ctx.db->query<model::CppHeaderInclusion>(
    odb::query<model::CppHeaderInclusion>::included == file_->id);

  for (auto inc : inclusions)
  {
    model::FilePtr loaded = inc.includer.load();
    if (!_ctx.fileStatus.count(loaded->path))
    {
      _ctx.fileStatus.emplace(loaded->path, IncrementalStatus::MODIFIED);
      LOG(debug) << "[cppparser] File modified: " << loaded->path;

      markByInclusion(loaded);
    }
  }
}

bool CppParser::parseByJson(
  const std::string& jsonFile_,
  std::size_t threadNum_)
{
  std::string errorMsg;

  std::unique_ptr<clang::tooling::JSONCompilationDatabase> compDb
    = clang::tooling::JSONCompilationDatabase::loadFromFile(
        jsonFile_, errorMsg,
        clang::tooling::JSONCommandLineSyntax::Gnu);

  if (!errorMsg.empty())
  {
    LOG(error) << errorMsg;
    return false;
  }

  //--- Read the compilation commands compile database ---//

  std::vector<clang::tooling::CompileCommand> compileCommands =
    compDb->getAllCompileCommands();

  compileCommands.erase(
    std::remove_if(compileCommands.begin(), compileCommands.end(),
      [&](const clang::tooling::CompileCommand& c)
      {
        return !isSourceFile(c.Filename);
      }),
    compileCommands.end());

  std::size_t numCompileCommands = compileCommands.size();

  //--- Create a thread pool for the current commands ---//
  std::unique_ptr<
    util::JobQueueThreadPool<ParseJob>> pool =
    util::make_thread_pool<ParseJob>(
      threadNum_, [this, &numCompileCommands](ParseJob& job_)
      {
        const clang::tooling::CompileCommand& command = job_.command;

        LOG(info)
          << '(' << job_.index << '/' << numCompileCommands << ')'
          << " Parsing " << command.Filename;

        int error = this->parseWorker(command);

        if (error)
          LOG(warning)
            << '(' << job_.index << '/' << numCompileCommands << ')'
            << " Parsing " << command.Filename << " has been failed.";
      });

  //--- Push all commands into the thread pool's queue ---//
  std::size_t index = 0;

  for (const auto& command : compileCommands)
  {
    ParseJob job(command, ++index);

    auto hash = util::fnvHash(
      boost::algorithm::join(command.CommandLine, " "));

    if (_parsedCommandHashes.find(hash) != _parsedCommandHashes.end())
    {
      LOG(info)
        << '(' << index << '/' << numCompileCommands << ')'
        << " Already parsed " << command.Filename;

      continue;
    }

    //--- Add compile command hash ---//

    _parsedCommandHashes.insert(hash);

    //--- Push the job ---//

    pool->enqueue(job);
  }

  // Block execution until every job is finished.
  pool->wait();

  return true;
}

CppParser::~CppParser()
{
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("C++ Plugin");
    description.add_options()
      ("skip-doccomment",
       "If this flag is given the parser will skip parsing the documentation "
       "comments.");
    return description;
  }

  std::shared_ptr<CppParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CppParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
