#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/logutil.h>
#include <util/odbtransaction.h>

#include <parser/sourcemanager.h>
#include <javaparser/javaparser.h>

#include <memory>

#include <chrono>
#include <thread>

namespace cc
{
namespace parser
{
namespace java {

JavaParser::JavaParser(ParserContext &ctx_) : AbstractParser(ctx_) {
  _java_path = pr::search_path("java");
}

bool JavaParser::accept(const std::string &path_) {
  std::string ext = fs::extension(path_);
  return ext == ".json";
}

CompileCommand JavaParser::getCompileCommand(
  const pt::ptree::value_type &command_tree_) {
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
  const CompileCommand &compile_command_) {
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
  const CompileCommand &compile_command_,
  model::BuildActionPtr buildAction_,
  bool error_) {
  util::OdbTransaction transaction(_ctx.db);

  std::vector<model::BuildSource> sources;
  std::vector<model::BuildTarget> targets;

//  for (const auto& srcTarget : extractInputOutputs(compile_command_))
//  {
//    model::BuildSource buildSource;
//    buildSource.file = _ctx.srcMgr.getFile(srcTarget.first);
//    buildSource.file->parseStatus = error_
//      ? model::File::PSPartiallyParsed
//      : model::File::PSFullyParsed;
//    _ctx.srcMgr.updateFile(*buildSource.file);
//    buildSource.action = buildAction_;
//    sources.push_back(std::move(buildSource));
//
//    model::BuildTarget buildTarget;
//    buildTarget.file = _ctx.srcMgr.getFile(srcTarget.second);
//    buildTarget.action = buildAction_;
//    if (buildTarget.file->type != model::File::BINARY_TYPE)
//    {
//      buildTarget.file->type = model::File::BINARY_TYPE;
//      _ctx.srcMgr.updateFile(*buildTarget.file);
//    }
//
//    targets.push_back(std::move(buildTarget));
//  }
//
//  _ctx.srcMgr.persistFiles();
//
//  transaction([&, this] {
//    for (model::BuildSource buildSource : sources)
//      _ctx.db->persist(buildSource);
//    for (model::BuildTarget buildTarget : targets)
//      _ctx.db->persist(buildTarget);
//  });
}

bool JavaParser::parse() {
  for (const std::string &path
    : _ctx.options["input"].as<std::vector<std::string>>()) {
    if (accept(path)) {
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
        [](pt::ptree::value_type &command_tree_)
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
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
      LOG(info) << "Waiting done.";

      // std::string line;
      // while (c.running() && std::getline(is, line) && !line.empty()) {
      //   LOG(info) << line;
      //   if (line == "Starting the simple server...") {
      //     break;
      //   }
      // }

      JavaParserServiceHandler serviceHandler;

      // Process Java files
      for (pt::ptree::value_type &command_tree_ : _pt_filtered) {
        CompileCommand compile_command =
          getCompileCommand(command_tree_);

        model::FilePtr filePtr = _ctx.srcMgr.getFile(compile_command.file);
        filePtr -> type = "JAVA";
        model::BuildActionPtr buildAction = addBuildAction(compile_command);

        // Thrift functions

        // Set arguments of the current command (prepare for parsing)
        serviceHandler.setArgs(compile_command);
        // Run Java parser
        serviceHandler.parseFile(filePtr -> id, file_index);

        addCompileCommand(compile_command, buildAction);
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

std::shared_ptr<JavaParser> make(ParserContext &ctx_) {
  return std::make_shared<JavaParser>(ctx_);
}
}
#pragma clang diagnostic pop

} // java
} // parser
} // cc
