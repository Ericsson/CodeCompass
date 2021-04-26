#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

namespace bp = boost::process;
namespace fs = boost::filesystem;

class JavaParser : public AbstractParser {
public:
  JavaParser(ParserContext &ctx_);

  virtual ~JavaParser();

  virtual bool parse() override;

private:
  boost::filesystem::path java_path;
  bool accept(const std::string &path_);
};

} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
