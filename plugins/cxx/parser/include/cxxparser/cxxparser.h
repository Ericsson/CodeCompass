#ifndef CC_PARSER_CXXPARSER_H
#define CC_PARSER_CXXPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class CxxParser : public AbstractParser
{
public:
  CxxParser(ParserContext& ctx_);
  virtual ~CxxParser();  
  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override; 

private:
  bool parseByJson(const std::string& jsonFile_, std::size_t threadNum_) const;
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_CXXPARSER_H
