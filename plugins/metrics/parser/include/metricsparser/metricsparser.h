#ifndef CC_PARSER_METRICS_PARSER_H
#define CC_PARSER_METRICS_PARSER_H

#include <odb/core.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <parser/abstract_parser.h>
#include <parser/parser_context.h>

#include <util/parser/parseutil.h>

namespace cc
{
namespace parser
{

class MetricsParser : public AbstractParser
{
public:
  MetricsParser(ParserContext& ctx_);
  virtual std::string getName() const override;
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
  
  Loc getLocFromFile(model::FilePtr file) const;
  
  void setCommentTypes(
    model::File::Type& type,
    std::string& singleComment,
    std::string& multiCommentStart,
    std::string& multiCommentEnd) const;
  
  void eraseBlankLines(std::string& file) const;
  
  void eraseComments(
    std::string& file,
    const std::string& singleComment,
    const std::string& multiCommentStart,
    const std::string& multiCommentEnd) const;
  
  void persistLoc(const Loc& loc, model::FileId file);
  
private:
  std::shared_ptr<odb::database> _db;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_METRICS_PARSER_H
