#include <dummyparser/dummyparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

DummyParser::DummyParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}

std::vector<std::string> DummyParser::getDependentParsers()  const
{
  return std::vector<std::string>{};
}

bool DummyParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".dummy";
}

bool DummyParser::parse()
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

DummyParser::~DummyParser()
{
}

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

  std::shared_ptr<DummyParser> make(ParserContext& ctx_)
  {
    return std::make_shared<DummyParser>(ctx_);
  }
}

} // parser
} // cc
