#ifndef CC_PARSER_CXXPARSER_H
#define CC_PARSER_CXXPARSER_H

#include <map>
#include <vector>
#include <unordered_set>
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
  /**
   * This function gets the input-output pairs from the compile command.
   *
   * If the compile command comtains C/C++ source file(s) and the output is set
   * by -o flag then the output file will be mapped to these source files.
   * Otherwise if no -o flag given but -c is given then the output files will
   * have the same name as the source file but with .o extension.  If no -c and
   * -o provided then the output file name will be a.out.  If the compile
   * command contains no source files then the function returns an empty map.
   */
  std::map<std::string, std::string> extractInputOutputs(
    const clang::tooling::CompileCommand& command_) const;

  void addCompileCommand(const clang::tooling::CompileCommand& command_);
  bool isParsed(const clang::tooling::CompileCommand& command_);
  bool isSourceFile(const std::string& file_) const;
  bool parseByJson(const std::string& jsonFile_, std::size_t threadNum_);
  void worker();

  std::vector<clang::tooling::CompileCommand> _compileCommands;
  std::size_t _index;

  std::unordered_set<std::uint64_t> _parsedCommandHashes;
};
  
} // parser
} // cc

#endif // CC_PARSER_CXXPARSER_H
