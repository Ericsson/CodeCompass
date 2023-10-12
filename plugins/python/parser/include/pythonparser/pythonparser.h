#ifndef CC_PARSER_PYTHONPARSER_H
#define CC_PARSER_PYTHONPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class PythonParser : public AbstractParser
{
public:
  PythonParser(ParserContext& ctx_);
  virtual ~PythonParser();
  virtual bool parse() override;
private:
  bool accept(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PARSER_PYTHONPARSER_H
