#ifndef CC_PARSER_CXXPARSER_H
#define CC_PARSER_CXXPARSER_H

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <model/buildaction.h>

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
  virtual void markModifiedFiles() override;
  /**
   * Maintains and cleans up the database in preparation of incremental parsing.
   *
   * A graph is constructed from the files to be cleaned up along the inclusion
   * relations. Then the nodes of the graph are sorted topologically.
   * In each iteration the leaf nodes of the graph are cleaned up (paralelly),
   * which guarantees that no file is cleaned up before its dependants.
   *
   * @return Returns true if the cleanup succeeded, false otherwise.
   */
  virtual bool cleanupDatabase() override;
  virtual bool parse() override;

private:
  /**
   * A single build command's cc::util::JobQueueThreadPool job.
   */
  struct ParseJob
  {
    /**
     * The build command itself. This is given to CppParser::worker.
     */
    std::reference_wrapper<const clang::tooling::CompileCommand> command;

    /**
     * The # of the build command in the compilation command database.
     */
    std::size_t index;

    ParseJob(const clang::tooling::CompileCommand& command, std::size_t index)
      : command(command), index(index)
    {}

    ParseJob(const ParseJob&) = default;
  };

  struct CleanupJob
  {
    /**
     * The path of the file to be cleaned up.
     */
    std::string path;

    /**
     * The # of the cleanup job.
     */
    std::size_t index;

    CleanupJob(const std::string& path, std::size_t index)
      : path(path), index(index)
    {}
  };


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

  model::BuildActionPtr addBuildAction(
    const clang::tooling::CompileCommand& command_);

  void addCompileCommand(
    const clang::tooling::CompileCommand& command_,
    model::BuildActionPtr buildAction_,
    bool error_ = false);

  bool isParsed(const clang::tooling::CompileCommand& command_);
  bool isSourceFile(const std::string& file_) const;
  bool isNonSourceFlag(const std::string& arg_) const;
  bool parseByJson(const std::string& jsonFile_, std::size_t threadNum_);
  int parseWorker(const clang::tooling::CompileCommand& command_);
  
  void initBuildActions();
  void markByInclusion(model::FilePtr file_);
  std::vector<std::vector<std::string>> createCleanupOrder();
  bool cleanupWorker(const std::string& path_);

  std::unordered_set<std::uint64_t> _parsedCommandHashes;

};
  
} // parser
} // cc

#endif // CC_PARSER_CXXPARSER_H
