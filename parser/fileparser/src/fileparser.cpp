#include <fileparser/fileparser.h>

namespace cc
{
namespace parser
{

std::shared_ptr<IFileParser> FileParser::getParser(const std::string& path_)
{
  for (auto fp : _fileParsers)
  {
    if (fp->accept(path_))
    {
      return fp;
    }
  }

  return {};
}

std::future<ParseResult> FileParser::parse(
  ParseProps parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::string& file_,
  const std::string& targetFile_,
  const std::vector<std::string>& opts_)
{
  auto fp = getParser(file_);
  
  if(fp)
  {
    return fp->parse(parseProps_, buildAction_, srcMgr_, file_, targetFile_,
     opts_);
  }

  std::promise<ParseResult> prom;
  prom.set_value(PARSE_FAIL);
  return prom.get_future();
}

bool FileParser::hasParser(const std::string& file_)
{  
  return getParser(file_) != nullptr;
}

} // parser
} // cc
