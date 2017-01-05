#ifndef CXXPARSER_NULLPARSER_H
#define CXXPARSER_NULLPARSER_H

#include <string>
#include <vector>
#include <fileparser/fileparser.h>


namespace cc
{
namespace parser
{

class CXXNullParser : public IFileParser
{
public:
  virtual bool accept(const std::string& path_) override;

  virtual std::future<ParseResult> parse(
    ParseProps parseProps,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::string& path_,
    const std::string& targetPath_,
    const std::vector<std::string>& opts_ ) override;

  virtual std::future<ParseResult> parse(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets&) override;

  std::string getDefaultTargetPath(const std::string& srcPath) override;
};

} // parser
} // cc

#endif
