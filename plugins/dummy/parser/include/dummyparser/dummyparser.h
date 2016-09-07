#ifndef CC_PARSER_DUMMYPARSER_H
#define CC_PARSER_DUMMYPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class DummyParser : public AbstractParser
{
public:
  DummyParser(ParserContext& ctx_);
  virtual ~DummyParser();
  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override; 
private:
  bool accept(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_DUMMYPARSER_H
