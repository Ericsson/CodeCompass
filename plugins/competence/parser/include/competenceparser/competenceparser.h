#ifndef CC_PARSER_DUMMYPARSER_H
#define CC_PARSER_DUMMYPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class CompetenceParser : public AbstractParser
{
public:
  CompetenceParser(ParserContext& ctx_);
  virtual ~CompetenceParser();
  virtual bool parse() override;
private:
  bool accept(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_DUMMYPARSER_H
