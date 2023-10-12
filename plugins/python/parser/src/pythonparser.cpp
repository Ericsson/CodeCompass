#include <pythonparser/pythonparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

PythonParser::PythonParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}

bool PythonParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".dummy";
}

bool PythonParser::parse()
{        
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(accept(path))
    {
      LOG(info) << "DummyParser parse path: " << path;
    }
  }
  return true;
}

PythonParser::~PythonParser()
{
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Python Plugin");

    description.add_options()
        ("dummy-arg", po::value<std::string>()->default_value("Dummy arg"),
          "This argument will be used by the dummy parser.");

    return description;
  }

  std::shared_ptr<PythonParser> make(ParserContext& ctx_)
  {
    return std::make_shared<PythonParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
