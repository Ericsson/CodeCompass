#include <dummyparser/dummyparser.h>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <iostream>
#include <memory>

namespace cc
{
namespace parser
{

DummyParser::DummyParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}
  
std::string DummyParser::getName() const
{
  return "dummyparser";
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
      BOOST_LOG_TRIVIAL(info) << "DummyParser parse path: " << path;
    }
  }
  return true;
}

DummyParser::~DummyParser()
{
}

extern "C"
{
  std::shared_ptr<DummyParser> make(ParserContext& ctx_)
  {
    return std::make_shared<DummyParser>(ctx_);
  }
}

} // parser
} // cc
