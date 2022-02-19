#include <csharpparser/csharpparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

CsharpParser::CsharpParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _threadNum = _ctx.options["jobs"].as<int>();
}

bool CsharpParser::acceptCompileCommands_dir(const std::string& path_)
{
  return fs::is_directory(path_);
}

bool CsharpParser::parse()
{        
  bool succes = true;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(acceptCompileCommands_dir(path))
    {
      LOG(info) << "CsharpParser parse path: " << path;
      succes = succes && parseCompileCommands_dir(path);
    }
  }
  return true;
}

bool CsharpParser::parseCompileCommands_dir(const std::string& path_) {
  fs::path csharp_path = fs::system_complete("../lib/csharp");

  std::future<std::string> log;

  std::string command("./CSharpParser ");
  command.append(path_);

  int result = bp::system(command, bp::start_dir(csharp_path), bp::std_out > log);

  LOG(info) << log.get();

  return result == 0;
}


CsharpParser::~CsharpParser()
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
    boost::program_options::options_description description("Dummy Plugin");

    description.add_options()
        ("dummy-arg", po::value<std::string>()->default_value("Dummy arg"),
          "This argument will be used by the dummy parser.");

    return description;
  }

  std::shared_ptr<CsharpParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CsharpParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
