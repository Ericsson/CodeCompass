#ifndef CC_PARSER_DUMMYPARSER_H
#define CC_PARSER_DUMMYPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;

class CsharpParser : public AbstractParser
{
public:
  DummyParser(ParserContext& ctx_);
  virtual ~DummyParser();
  virtual bool parse() override;
private:
  int _numCompileCommands;
  int _threadNum;
  bool acceptCompileCommands(const std::string& path_);
  bool parseCompileCommands(const std::string& path_);
  int CppParser::parseWorker(const clang::tooling::CompileCommand& command_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_DUMMYPARSER_H
