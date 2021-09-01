#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <model/buildaction.h>
#include <model/buildsourcetarget.h>
#include <model/file.h>

#include <util/logutil.h>

#include <parser/sourcemanager.h>
#include <javaparser/javaparser.h>

#include <memory>

namespace cc
{
namespace parser
{

namespace bp = boost::process;
namespace bt = boost::property_tree;

JavaParser::JavaParser(ParserContext &ctx_) : AbstractParser(ctx_)
{
  _java_path = bp::search_path("java");
}

bool JavaParser::accept(const std::string &path_)
{
  std::string ext = bs::extension(path_);
  return ext == ".json";
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

      bt::ptree pt;
      bt::read_json(path, pt);

       for (bt::ptree::value_type &c : pt)
       {
         model::BuildSource buildSource;
         buildSource.file =
           _ctx.srcMgr.getFile(c.second.get<std::string>("file"));
       }

      bp::system(
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
