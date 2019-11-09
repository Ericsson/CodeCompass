#ifndef CC_PARSER_GITPARSER_H
#define CC_PARSER_GITPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <util/parserutil.h>

namespace cc
{
namespace parser
{

class GitParser : public AbstractParser
{
public:
  GitParser(ParserContext& ctx_);
  virtual ~GitParser();
  virtual bool parse() override;
private:
  util::DirIterCallback getParserCallback();
};

} // parser
} // cc

#endif //CC_PARSER_GITPARSER_H
