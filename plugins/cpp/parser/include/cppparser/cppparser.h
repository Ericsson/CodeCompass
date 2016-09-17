#ifndef CC_PARSER_CXXPARSER_H
#define CC_PARSER_CXXPARSER_H

#include <mutex>

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{
  
class CppParser : public AbstractParser
{
public:
  CppParser(ParserContext& ctx_);
  virtual ~CppParser();  
  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override; 

private:
  bool parseByJson(const std::string& jsonFile_, std::size_t threadNum_);
  void worker();

  std::vector<clang::tooling::CompileCommand> _compileCommands;
  std::size_t _index;
  std::mutex _mutex;
};
  
} // parser
} // cc

#endif // CC_PARSER_CXXPARSER_H
