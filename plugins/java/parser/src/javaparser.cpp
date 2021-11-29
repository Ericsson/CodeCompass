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

  for (int i = 0; i < threadNum; ++i) {
    javaServiceHandlers.push_back(
      std::make_shared<JavaParserServiceHandler>(JavaParserServiceHandler())
    );
  }
}

bool JavaParser::accept(const std::string& path_) {
  std::string ext = fs::extension(path_);
  return ext == ".json" || ext == ".jar";
}

void JavaParser::initializeWorkers() {
  int worker_num = 0;
  for (auto &handler : javaServiceHandlers) {
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
  JavaParser::findFreeWorker(int timeout_in_ms)
{
  chrono::steady_clock::time_point begin = chrono::steady_clock::now();
  chrono::steady_clock::time_point current = begin;
  float elapsed_time = 0;

  while (elapsed_time < timeout_in_ms) {
    for (auto &handler: javaServiceHandlers) {
      if (handler->is_free) {
        handler->is_free = false;
        return handler;
      }
    }

    current = chrono::steady_clock::now();
    elapsed_time =
      chrono::duration_cast<chrono::milliseconds>(current - begin).count();
  }

  throw TimeoutException("Could not find free Java Worker");
}

CompileCommand JavaParser::getCompileCommand(
  const pt::ptree::value_type& command_tree_) {
  CompileCommand compile_command;

  compile_command.directory =
    command_tree_.second.get<std::string>("directory");
  compile_command.command =
    command_tree_.second.get<std::string>("command");
  compile_command.file =
    command_tree_.second.get<std::string>("file");

  return compile_command;
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
  model::File::ParseStatus parseStatus_)
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
  for (const std::string& path
    : _ctx.options["input"].as<std::vector<std::string>>()) {
    if (accept(path)) {
      pt::ptree _pt;
      pt::read_json(path, _pt);
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
            auto ext = fs::extension(
              command_tree_.second.get<std::string>("file"));
            return ext == ".java";
        });

      const int numCompileCommands = _pt_filtered.size();

      std::vector<std::string> _java_args{
        "-DrawDbContext=" + _ctx.options["database"].as<std::string>(),
        "-DthreadNum=" + std::to_string(threadNum),
        "-jar",
        "../lib/java/javaparser.jar"
      };

      c = pr::child(_java_path, _java_args, pr::std_out > stdout);

      initializeWorkers();

      //--- Create a thread pool for the current commands ---//

      std::unique_ptr<
      util::JobQueueThreadPool<ParseJob>> pool =
        util::make_thread_pool<ParseJob>(
          threadNum,
          [this, &numCompileCommands](ParseJob& job_)
          {
            CompileCommand command =
              getCompileCommand(job_.command_tree);
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
              std::to_string(numCompileCommands) + ")";

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
            }  catch (JavaBeforeParseException& ex) {
              LOG(warning) <<
                file_counter_str << " " << "Parsing " <<
                command.file << " has been failed before the start";

              LOG(warning) << ex.message;
              parseStatus = model::File::PSNone;
            } catch (JavaParseException& ex) {
              LOG(warning) <<
                file_counter_str << " " << "Parsing " <<
                command.file << " has been failed during parsing";

              LOG(warning) << ex.message;
                parseStatus = model::File::PSPartiallyParsed;
            }


            addCompileCommand(
              parseResult.cmdArgs, buildAction, parseStatus);

            serviceHandler->is_free = true;
          });

      //--- Process Java files ---//

      LOG(info) << "JavaParser parse path: " << path;

      for (pt::ptree::value_type &command_tree_: _pt_filtered) {
        ParseJob job(command_tree_, ++file_index);

        //--- Push the job ---//

        pool->enqueue(job);
      }

      // Block execution until every job is finished.
      pool->wait();
    }
  }
  return true;
}

JavaParser::~JavaParser() {}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
boost::program_options::options_description getOptions() {
  boost::program_options::options_description description("Java Plugin");

  description.add_options()
    ("java-arg", po::value<std::string>()->default_value("Java arg"),
     "This argument will be used by the Java parser.");

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
