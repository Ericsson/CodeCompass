#include <iostream>
#include <cxxparser/nullparser.h>
#include <util/util.h>

namespace cc
{
namespace parser
{
  
bool CXXNullParser::accept(const std::string& path_)
{
  //TODO: accept all the standard C++ extensions
  return util::isExtension(path_, "cpp");
}

std::future<ParseResult> CXXNullParser::parse(
  ParseProps parseProps,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::string& path_,
  const std::string& targetPath_,
  const std::vector<std::string>& opts_)
{
  std::cerr << "INVOKED: " << path_ << std::endl;

  std::promise<ParseResult> res;
  res.set_value(PARSE_SUCCESS);
  return res.get_future();
}

std::future<ParseResult> CXXNullParser::parse(
  const ParseProps& parseProps_,
  model::BuildActionPtr buildAction_,
  SourceManager& srcMgr_,
  const std::vector<std::string>& opts_,
  const BuildSourceTargets&)
{
  std::promise<ParseResult> res;
  res.set_value(PARSE_SUCCESS);
  return res.get_future();
}

std::string CXXNullParser::getDefaultTargetPath(const std::string& srcPath)
{
  return "nothing";
}

} //parser
} //cc
