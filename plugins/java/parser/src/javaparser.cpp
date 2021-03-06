#include <javaparser/javaparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

JavaParser::JavaParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}

bool JavaParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".java";
}

bool JavaParser::parse()
{        
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(accept(path))
    {
      LOG(info) << "JavaParser parse path: " << path;
    }
  }
  return true;
}

JavaParser::~JavaParser()
{
}

/* These two methods are used by the plugin manager to allow dynamic loading
   of CodeCompass Parser plugins. Clang (>= version 6.0) gives a warning that
   these C-linkage specified methods return types that are not proper from a
   C code.

   These codes are NOT to be called from any C code. The C linkage is used to
   turn off the name mangling so that the dynamic loader can easily find the
   symbol table needed to set the plugin up.
*/
// When writing a plugin, please do NOT copy this notice to your code.
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

  std::shared_ptr<JavaParser> make(ParserContext& ctx_)
  {
    return std::make_shared<JavaParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
