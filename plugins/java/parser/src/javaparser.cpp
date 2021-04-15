#include <parser/javaparser.h>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

JavaParser::JavaParser(ParserContext &ctx_) : AbstractParser(ctx_) {
}

bool JavaParser::accept(const std::string &path_) {
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".java";
}

bool JavaParser::parse() {
  namespace bp = boost::process;
  for (std::string path : _ctx.options["input"].as < std::vector < std::string >> ()) {
    LOG(info) << "===================";
    std::string command = "java -jar ../lib/java/javaparser.jar " + path;
    bp::system(command);
    LOG(info) << "===================";
    if (accept(path)) {
      LOG(info) << "JavaParser parse path: " << path;
    }
  }
  return true;
}

JavaParser::~JavaParser() {
}

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

std::shared_ptr <JavaParser> make(ParserContext &ctx_) {
  return std::make_shared<JavaParser>(ctx_);
}
}
#pragma clang diagnostic pop

} // parser
} // cc
