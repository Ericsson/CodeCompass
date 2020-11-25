#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class JavaParser : public AbstractParser
{
public:
  JavaParser(ParserContext& ctx_);
  virtual ~JavaParser();
  virtual bool parse() override;
private:
  bool accept(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_JAVAPARSER_H
