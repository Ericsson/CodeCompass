#ifndef CC_PARSER_CSHARPPARSER_H
#define CC_PARSER_CSHARPPARSER_H

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;
namespace bp = boost::process;
namespace pt = boost::property_tree;

class CsharpParser : public AbstractParser
{
public:
  CsharpParser(ParserContext& ctx_);
  virtual ~CsharpParser();
  virtual bool parse() override;
private:
  int _numCompileCommands;
  int _threadNum;
  bool acceptCompileCommands_dir(const std::string& path_);
  bool parseCompileCommands_dir(const std::string& path_);
};
  
} // parser
} // cc

#endif // CC_PLUGINS_PARSER_DUMMYPARSER_H
