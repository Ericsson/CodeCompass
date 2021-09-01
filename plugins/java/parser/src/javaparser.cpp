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

namespace cc
{
namespace parser
{

JavaParser::JavaParser(ParserContext &ctx_) : AbstractParser(ctx_)
{
  _java_path = pr::search_path("java");
}

bool JavaParser::accept(const std::string &path_)
{
  std::string ext = fs::extension(path_);
  return ext == ".json";
}

model::BuildActionPtr JavaParser::addBuildAction(
  const pt::ptree::value_type& command_)
{
  util::OdbTransaction transaction(_ctx.db);

  model::BuildActionPtr buildAction(new model::BuildAction);

  std::string extension = fs::extension(
    command_.second.get<std::string>("file"));

  buildAction->command = command_.second.get<std::string>("command");
  buildAction->type
    = extension == ".class"
    ? model::BuildAction::Link
    : model::BuildAction::Compile;

  transaction([&, this]{ _ctx.db->persist(buildAction); });

  return buildAction;
}

void JavaParser::addCompileCommand(
  const pt::ptree::value_type& command_,
  model::BuildActionPtr buildAction_,
  bool error_)
{
//  util::OdbTransaction transaction(_ctx.db);
//
//  std::vector<model::BuildSource> sources;
//  std::vector<model::BuildTarget> targets;
//
//  for (const auto& srcTarget : extractInputOutputs(command_))
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

bool JavaParser::parse()
{
  for (const std::string& path
    : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if (accept(path))
    {
      // _ctx.srcMgr.getFile(path); vagy nem path, hanem a java fájlok.
      // Persist is megvan ezzel a függvénnyel.
      // ODB transactiont kell csinálni majd.
      // transaction-nel lekérem az összes build actiont,
      // és az alapján persistelem a fájlokat, cppparserben minta

      pt::ptree _pt;
      pt::read_json(path, _pt);

      for (pt::ptree::value_type& command_ : _pt)
      {
        model::BuildActionPtr buildAction = addBuildAction(command_);

        // model::BuildSource buildSource;
        // buildSource.file =
        //   _ctx.srcMgr.getFile(
        //     command_.second.get<std::string>("file"));
      }

      pr::system(
              _java_path, "-jar", "../lib/java/javaparser.jar",
              _ctx.options["database"].as<std::string>(), path
      );
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
boost::program_options::options_description getOptions()
{
  boost::program_options::options_description description("Java Plugin");

  description.add_options()
  ("java-arg", po::value<std::string>()->default_value("Java arg"),
   "This argument will be used by the Java parser.");

  return description;
}

std::shared_ptr <JavaParser> make(ParserContext &ctx_)
{
  return std::make_shared<JavaParser>(ctx_);
}
}
#pragma clang diagnostic pop

} // parser
} // cc
