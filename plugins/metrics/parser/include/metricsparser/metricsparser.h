#ifndef CC_PARSER_METRICS_PARSER_H
#define CC_PARSER_METRICS_PARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <util/parserutil.h>

namespace cc
{
namespace parser
{

class MetricsParser : public AbstractParser
{
public:
  MetricsParser(ParserContext& ctx_);
  virtual std::vector<std::string> getDependentParsers() const override;  
  virtual bool parse() override;

private:
  util::DirIterCallback getParserCallback();

  struct Loc
  {
    unsigned originalLines;
    unsigned nonblankLines;
    unsigned codeLines;
  };

  Loc getLocFromFile(model::FilePtr file_) const;

  void setCommentTypes(
    model::FileTypePtr& filetype_,
    std::string& singleComment_,
    std::string& multiCommentStart_,
    std::string& multiCommentEnd_) const;

  void eraseBlankLines(std::string& file) const;

  void eraseComments(
    std::string& file_,
    const std::string& singleComment_,
    const std::string& multiCommentStart_,
    const std::string& multiCommentEnd_) const;

  void persistLoc(const Loc& loc_, model::FileId file_);
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_METRICS_PARSER_H
