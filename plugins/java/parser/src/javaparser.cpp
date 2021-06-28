#include <unordered_set>

#include <parser/javaparser.h>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

JavaParser::JavaParser(ParserContext &ctx_) : AbstractParser(ctx_)
{
  _java_path = bp::search_path("java");
}

bool JavaParser::accept(const std::string &path_)
{
  std::string ext = fs::extension(path_);
  return ext == ".json";
}

bool JavaParser::parse()
{
  for (const std::string& path
    : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if (accept(path))
    {
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
