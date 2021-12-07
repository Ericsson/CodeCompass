#include <unordered_set>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/buildlog.h>
#include <model/buildlog-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/logutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <parser/sourcemanager.h>
#include <javaparser/javaparser.h>

#include <memory>
#include <thread>

namespace cc
{
namespace parser
{
namespace java
{

// Initialize static members
std::stringstream JavaParserServiceHandler::thrift_ss;
bool JavaParserServiceHandler::server_started = false;

JavaParser::JavaParser(ParserContext& ctx_) : AbstractParser(ctx_) {
  _java_path = pr::search_path("java");
  _unzip_path = pr::search_path("unzip");
  _threadNum = _ctx.options["jobs"].as<int>();

  //--- Create a thread pool to process commands ---//

  _parsePool =
    util::make_thread_pool<ParseJob>(
      _threadNum,[this](ParseJob& job_)
      {
        const CompileCommand command = job_.command;
        std::shared_ptr<JavaParserServiceHandler> serviceHandler;

        try {
          serviceHandler = findFreeWorker(15000);
        } catch (TimeoutException& ex) {
          LOG(error) <<
            "Operation timeout, could not find free "
            "Java worker to process " << command.file << "!";
          return;
        }

        std::string file_counter_str =
          "(" + std::to_string(job_.index) + "/" +
          std::to_string(_numCompileCommands) + ")";

        LOG(info) <<
          file_counter_str << " " << "Parsing " << command.file;

        model::FilePtr filePtr = _ctx.srcMgr.getFile(command.file);
        filePtr -> type = "JAVA";
        model::BuildActionPtr buildAction = addBuildAction(command);
        model::File::ParseStatus parseStatus =
          model::File::PSFullyParsed;
        ParseResult parseResult;

        //--- Run Java parser ---//

        try {
          serviceHandler->parseFile(
            parseResult, command, filePtr->id, file_counter_str);
          parseStatus =
            addBuildLogs(parseResult.buildLogs, command.file);

          if (parseResult.errorDueParsing) {
            LOG(warning) <<
              file_counter_str << " " << "Parsing " <<
              command.file << " had one or more errors during parsing";

            if (parseStatus == model::File::ParseStatus::PSFullyParsed) {
              parseStatus = model::File::ParseStatus::PSPartiallyParsed;
            }
          }

          addCompileCommand(
            parseResult.cmdArgs, buildAction, parseStatus);

        } catch (JavaBeforeParseException& ex) {
          LOG(warning) <<
            file_counter_str << " " << "Parsing " <<
            command.file << " has been failed before the start";
          LOG(warning) << ex.message;
        }

        serviceHandler->setFree();
      });

  for (int i = 0; i < _threadNum; ++i) {
    _javaServiceHandlers.push_back(
      std::make_shared<JavaParserServiceHandler>(JavaParserServiceHandler())
    );
  }
}

bool JavaParser::acceptCompileCommands(const std::string& path_) {
  std::string ext = fs::extension(path_);
  return ext == ".json";
}

bool JavaParser::acceptJar(const std::string& path_) {
  std::string ext = fs::extension(path_);
  return ext == ".jar";
}

bool JavaParser::acceptClass(const std::string& path_) {
  std::string ext = fs::extension(path_);
  return ext == ".class";
}

bool JavaParser::rejectInnerClass(const std::string& path_) {
  std::string name = fs::basename(path_);
  return name.find('$') == std::string::npos;
}

void JavaParser::startAndConnectToJavaProcess() {
  std::vector<std::string> _java_args{
    "-DrawDbContext=" + _ctx.options["database"].as<std::string>(),
    "-DthreadNum=" + std::to_string(_threadNum),
    "-jar",
    "../lib/java/javaparser.jar"
  };

  _c = pr::child(_java_path, _java_args, pr::std_out > stdout);

  initializeWorkers();
}

void JavaParser::initializeWorkers() {
  int worker_num = 0;
  for (auto &handler : _javaServiceHandlers) {
    try {
      handler->getClientInterface(25000, ++worker_num);
    } catch (TransportException& ex) {
      LOG(error) <<
        "[javaparser] Failed to start worker (" << worker_num << ")!";
      throw ex;
    }
  }
}

std::shared_ptr<JavaParserServiceHandler>&
  JavaParser::findFreeWorker(int timeout_in_ms_)
{
  chrono::steady_clock::time_point begin = chrono::steady_clock::now();
  chrono::steady_clock::time_point current;
  float elapsed_time = 0;

  while (elapsed_time < timeout_in_ms_) {
    for (auto &handler: _javaServiceHandlers) {
      if (handler->isFree()) {
        handler->reserve();
        return handler;
      }
    }

    current = chrono::steady_clock::now();
    elapsed_time =
      chrono::duration_cast<chrono::milliseconds>(current - begin).count();
  }

  throw TimeoutException("Could not find free Java Worker");
}

CompileCommand JavaParser::getCompileCommandFromJson(
  const pt::ptree::value_type& command_tree_)
{
  CompileCommand compile_command;

  compile_command.directory =
    command_tree_.second.get<std::string>("directory");
  compile_command.command =
    command_tree_.second.get<std::string>("command");
  compile_command.file =
    command_tree_.second.get<std::string>("file");

  return compile_command;
}

CompileCommand JavaParser::getCompileCommandForDecompiledFile(
  const std::string& filePath_, const std::string& classpath_,
  const std::string& sourcepath_)
{
  CompileCommand compile_command;
  std::ostringstream javacCommand;
  javacCommand << "javac" << " -sourcepath " << sourcepath_ <<
    " -classpath " << classpath_ << " " << filePath_;

  compile_command.directory = fs::path(filePath_).parent_path().string();
  compile_command.command = javacCommand.str();
  compile_command.file = filePath_;

  return compile_command;
}

std::string JavaParser::getClasspathFromMetaInf(
  const fs::path& root_, const fs::path& originalDir_)
{
  std::ostringstream rawClasspath;
  std::ostringstream classpath;
  std::vector<std::string> classpathVec;
  fs::path manifestPath = root_ / "META-INF" / "MANIFEST.MF";

  classpath << root_.string();

  if (!fs::exists(manifestPath)) {
    return classpath.str();
  }

  fs::ifstream fileHandler(manifestPath);
  std::string line;
  bool cpFound = false;

  while (
    getline(fileHandler, line) &&
    !(cpFound && ba::contains(line, ":")))
  {
    if (
      (!cpFound && ba::starts_with(line, "Class-Path:")) ||
      (cpFound && !ba::contains(line, ":")))
    {
      line = line.substr(0, line.size() -1);
      if (!cpFound) {
        ba::replace_first(line, "Class-Path:", "");
        rawClasspath << line;
        cpFound = true;
      }

      rawClasspath << line.substr(1);
    }
  }

  ba::split_regex(
    classpathVec, rawClasspath.str(), boost::regex("([ ]+)"));

  for (std::string& cp : classpathVec) {
    if (!cp.empty()) {
      classpath << ":" << (originalDir_ / cp).string();
    }
  }

  return classpath.str();
}

model::BuildActionPtr JavaParser::addBuildAction(
  const CompileCommand& compile_command_)
{
  util::OdbTransaction transaction(_ctx.db);
  model::BuildActionPtr buildAction(new model::BuildAction);
  std::string extension = fs::extension(compile_command_.file);

  buildAction -> command = compile_command_.command;
  buildAction -> type
    = extension == ".class"
      ? model::BuildAction::Link
      : model::BuildAction::Compile;

  transaction([&, this] { _ctx.db->persist(buildAction); });

  return buildAction;
}

void JavaParser::addCompileCommand(
  const CmdArgs& cmd_args_,
  model::BuildActionPtr buildAction_,
  model::File::ParseStatus& parseStatus_)
{
  util::OdbTransaction transaction(_ctx.db);
  std::vector<model::BuildTarget> targets;

  model::BuildSource buildSource;
  buildSource.file = _ctx.srcMgr.getFile(cmd_args_.filepath);
  buildSource.file->parseStatus = parseStatus_;
  _ctx.srcMgr.updateFile(*buildSource.file);
  buildSource.action = buildAction_;

  for (const std::string& target : cmd_args_.bytecodesPaths) {
    model::BuildTarget buildTarget;
    buildTarget.file = _ctx.srcMgr.getFile(target);
    buildTarget.action = buildAction_;
    if (buildTarget.file->type != model::File::BINARY_TYPE) {
      buildTarget.file->type = model::File::BINARY_TYPE;
      _ctx.srcMgr.updateFile(*buildTarget.file);
    }

    targets.push_back(std::move(buildTarget));
  }

  _ctx.srcMgr.persistFiles();

  transaction([&, this] {
    _ctx.db->persist(buildSource);
    for (model::BuildTarget buildTarget : targets)
      _ctx.db->persist(buildTarget);
  });
}

model::File::ParseStatus JavaParser::addBuildLogs(
  const std::vector<core::BuildLog>& buildLogs_, const std::string& file_)
{
  util::OdbTransaction transaction(_ctx.db);
  std::vector<model::BuildLog> logs;
  model::File::ParseStatus parseStatus =
    model::File::ParseStatus::PSFullyParsed;

  for (const core::BuildLog& log : buildLogs_) {
    model::BuildLog buildLog;

    buildLog.location.file = _ctx.srcMgr.getFile(file_);
    buildLog.log.message = log.message;
    buildLog.location.range.start.line = log.range.startpos.line;
    buildLog.location.range.start.column = log.range.startpos.column;
    buildLog.location.range.end.line = log.range.startpos.line;
    buildLog.location.range.end.column = log.range.endpos.column;

    switch (log.messageType) {
      case core::MessageType::FatalError:
        buildLog.log.type = model::BuildLogMessage::FatalError;

        if (parseStatus > model::File::ParseStatus::PSNone) {
          parseStatus = model::File::ParseStatus::PSNone;
        }
        break;
      case core::MessageType::Error:
        buildLog.log.type = model::BuildLogMessage::Error;

        if (parseStatus > model::File::ParseStatus::PSPartiallyParsed) {
          parseStatus = model::File::ParseStatus::PSPartiallyParsed;
        }
        break;
      case core::MessageType::Warning:
        buildLog.log.type = model::BuildLogMessage::Warning;
        break;
      case core::MessageType::Note:
        buildLog.log.type = model::BuildLogMessage::Note;
        break;
      default: // Just to suppress warning of uncovered enum constants
        break;
    }

    logs.push_back(std::move(buildLog));
  }

  transaction([&, this] {
    for (model::BuildLog buildLog : logs)
      _ctx.db->persist(buildLog);
  });

  return parseStatus;
}

bool JavaParser::parse() {
  bool success = true;

  for (const std::string& path
    : _ctx.options["input"].as<std::vector<std::string>>()) {
    if (acceptCompileCommands(path)) {
      bool parseSuccess = parseCompileCommands(path);

      if (success) {
        success = parseSuccess;
      }
    } else if (acceptJar(path)) {
      bool parseSuccess = parseJar(path);

      if (success) {
        success = parseSuccess;
      }
    }
  }
  return success;
}

bool JavaParser::parseCompileCommands(const std::string& path_) {
  pt::ptree _pt;
  pt::read_json(path_, _pt);
  pt::ptree _pt_filtered;
  pr::ipstream is;
  std::size_t file_index = 0;

  // Filter compile commands tree to contain only Java-related files
  std::copy_if (
    _pt.begin(),
    _pt.end(),
    std::back_inserter(_pt_filtered),
    [](pt::ptree::value_type& command_tree_)
    {
      auto ext =
        fs::extension(command_tree_.second.get<std::string>("file"));
      return ext == ".java";
    });

  _numCompileCommands = _pt_filtered.size();

  //--- Start Java Thrift server and connect to is via workers ---//

  startAndConnectToJavaProcess();

  //--- Process Java files ---//

  LOG(info) << "JavaParser parse path: " << path_;

  for (pt::ptree::value_type &command_tree_: _pt_filtered) {
    CompileCommand command =
      getCompileCommandFromJson(command_tree_);

    ParseJob job(command, ++file_index);

    //--- Push the job ---//

    _parsePool->enqueue(job);
  }

  // Block execution until every job is finished.
  _parsePool->wait();

  return true;
}

bool JavaParser::parseJar(const std::string& path_) {
  //--- Start Java Thrift server and connect to is via workers ---//

  startAndConnectToJavaProcess();

  std::vector<CompileCommand> commands = decompileJar(path_);
  _numCompileCommands = commands.size();
  std::size_t file_index = 0;

  for (const CompileCommand& command : commands) {
    ParseJob job(command, ++file_index);

    //--- Push the job ---//

    _parsePool->enqueue(job);
  }

  // Block execution until every job is finished.
  _parsePool->wait();

  return true;
}

std::vector<CompileCommand> JavaParser::decompileJar(const std::string& path_) {
  fs::path jarPath(path_);
  fs::path workspace(_ctx.options["workspace"].as<std::string>());
  fs::path project(_ctx.options["name"].as<std::string>());
  std::string decompiled = fs::basename(jarPath);
  fs::path decompiled_root = workspace / project / decompiled;
  std::vector<CompileCommand> commands;

  pr::system(
    _unzip_path, "-o", jarPath, "-d", decompiled_root
  );

  std::string classpath =
    getClasspathFromMetaInf(
      decompiled_root, jarPath.parent_path()
    );

  //--- Create a thread pool to decompile bytecodes ---//

  std::unique_ptr<
    util::JobQueueThreadPool<DecompileJob>> decompilePool =
    util::make_thread_pool<DecompileJob>(
      _threadNum,
      [this, &commands, &classpath, &decompiled_root]
      (DecompileJob& job_)
      {
        const std::string bytecodePath = job_.path;
        std::shared_ptr<JavaParserServiceHandler> serviceHandler;

        try {
          serviceHandler = findFreeWorker(15000);
        } catch (TimeoutException& ex) {
          LOG(error) <<
            "Operation timeout, could not find free "
            "Java worker to decompile " << bytecodePath << "!";
          return;
        }

        LOG(info) << "Decompiling " << bytecodePath;

        std::string javaFilePath;

        try {
          serviceHandler -> decompileClass(javaFilePath, bytecodePath);
          commands.push_back(
            getCompileCommandForDecompiledFile(
              javaFilePath, classpath, decompiled_root.string())
          );
        } catch (ClassDecompileException& ex) {
          LOG(warning) << "Decompiling " << bytecodePath << " has been failed";
          LOG(warning) << ex.message;
        }

        serviceHandler->setFree();
      });

  //-- Decompile bytecodes --//

  for (
    fs::directory_entry& entry :
    fs::recursive_directory_iterator(decompiled_root))
  {
    const std::string& bytecodePath = entry.path().string();

    if (acceptClass(bytecodePath) && rejectInnerClass(bytecodePath)) {
      DecompileJob job(bytecodePath);

      //--- Push the job ---//

      decompilePool->enqueue(job);
    }
  }

  // Block execution until every job is finished.
  decompilePool->wait();

  return commands;
}

JavaParser::~JavaParser() {}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
boost::program_options::options_description getOptions() {
  boost::program_options::options_description description("Java Plugin");

  // description.add_options()
  //   ("java-arg", po::value<std::string>()->default_value("Java arg"),
  //    "This argument will be used by the Java parser.");

  return description;
}

std::shared_ptr<JavaParser> make(ParserContext& ctx_) {
  return std::make_shared<JavaParser>(ctx_);
}
}
#pragma clang diagnostic pop

} // java
} // parser
} // cc
