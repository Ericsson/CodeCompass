#ifndef CC_PARSER_DUMMYPARSER_H
#define CC_PARSER_DUMMYPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class CppMetricsParser : public AbstractParser
{
public:
  CppMetricsParser(ParserContext& ctx_);
  virtual ~CppMetricsParser();
  virtual bool parse() override;
private:
  bool accept(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_CPPMETRICSPARSER_H
