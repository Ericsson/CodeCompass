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

// Initialize static member
std::stringstream JavaParserServiceHandler::thrift_ss;

JavaParser::JavaParser(ParserContext& ctx_) : AbstractParser(ctx_) {
  _java_path = pr::search_path("java");
}

bool JavaParser::accept(const std::string& path_) {
  std::string ext = fs::extension(path_);
  return ext == ".json";
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
  const std::vector<core::BuildLog>& buildLogs_, std::string& file_)
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
      JavaParserServiceHandler serviceHandler;
      int file_index = 1;
      pt::ptree _pt;
      pt::read_json(path, _pt);
      pt::ptree _pt_filtered;
      pr::ipstream is;

      // Filter compile commands tree to contain only Java-related files
      std::copy_if (
        _pt.begin(),
        _pt.end(),
        std::back_inserter(_pt_filtered),
        [](pt::ptree::value_type& command_tree_)
        {
            auto ext = fs::extension(
              command_tree_.second.get<std::string>("file"));
            return ext == ".java" || ext == ".class";
        });

      std::vector<std::string> _java_args{
        "-DrawDbContext=" + _ctx.options["database"].as<std::string>(),
        "-Dsize=" + std::to_string(_pt_filtered.size()),
        "-jar",
        "../lib/java/javaparser.jar"
      };

      pr::child c(_java_path, _java_args, pr::std_out > stdout);

      try {
        serviceHandler.getClientInterface(25000);
      } catch (TransportException& ex) {
        LOG(error) << "[javaparser] Failed to parse!";
        return false;
      }

      // Process Java files
      for (pt::ptree::value_type& command_tree_ : _pt_filtered) {
        CompileCommand compile_command =
          getCompileCommand(command_tree_);
        model::FilePtr filePtr = _ctx.srcMgr.getFile(compile_command.file);
        filePtr -> type = "JAVA";
        model::BuildActionPtr buildAction = addBuildAction(compile_command);
        model::File::ParseStatus parseStatus = model::File::PSFullyParsed;
        std::vector<core::BuildLog> buildLogs;

        // Thrift functions

        // Set arguments of the current command (prepare for parsing)
        serviceHandler.setArgs(compile_command);
        // Run Java parser
        try {
          serviceHandler.parseFile(buildLogs, filePtr->id, file_index);
          parseStatus = addBuildLogs(buildLogs, compile_command.file);
        }  catch (JavaBeforeParseException& ex) {
          LOG(warning) << ex.message;
          parseStatus = model::File::PSNone;
        } catch (JavaParseException& ex) {
          LOG(warning) << ex.message;
          parseStatus = model::File::PSPartiallyParsed;
        }

        // Get arguments for the current file
        CmdArgs cmdArgs;
        serviceHandler.getArgs(cmdArgs);

        addCompileCommand(cmdArgs, buildAction, parseStatus);

        ++file_index;
      }
      LOG(info) << "JavaParser parse path: " << path;
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
