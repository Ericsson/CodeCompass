#ifndef CC_PARSER_HASKELLPARSER_H
#define CC_PARSER_HASKELLPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class HaskellParser : public AbstractParser
{
public:
  HaskellParser(ParserContext& ctx_);
  virtual ~HaskellParser();
  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override; 
private:
  bool accept(const std::string& path_);
  bool parseByJson(const std::string& jsonFile_);
};
  
} // parser
} // cc

#endif // CC_PARSER_HASKELLPARSER_H
