#ifndef GENERAL_PARSER_H
#define GENERAL_PARSER_H

#include <parser/abstract_parser.h>
#include <parser/parser_context.h>
#include <parser/projectparser.h>
#include <memory>

#include <parser/commondefs.h>
#include <parser/sourcemanager.h>
#include <parser/traversal.h>
#include <model/workspace.h>
#include <model/buildaction.h>
#include <model/buildparameter.h>
#include <model/buildsource.h>
#include <model/buildtarget.h>
#include <model/file.h>
#include <util/odbtransaction.h>

namespace cc
{
namespace parser
{
  
class GeneralParser : public AbstractParser
{
public:
  std::vector<std::string> getDependentParsers() override;
  bool parse(const std::vector<std::string>& path_, const ParserContext& ctx_) override; 
  std::string getName() override;
  
  struct Context
  {
    /**
      * The total number of source files (for the progress callback).
      */
    std::size_t sourceFileCount = 0;

    /**
     * If it set to false, than the traversals and persisting options will be
     * skipped.
     */
    bool partiallyParsed = false;
  };
  
  struct ParserTask
  {
    /**
     * True if the options filed contains a full command line. In that case
     * the files vector maybe empty or contains only one target file.
     */
    bool fullCommandLine = false;
    /**
     * Build action.
     */
    model::BuildActionPtr action;
    /**
     * Compiler options for this action.
     */
    std::vector<std::string> options;
    /**
     * Source-target pairs. These files will be parsed.
     */
    std::vector<std::tuple<model::BuildSourcePtr, model::BuildTargetPtr>> files;
  };  
  
  /**
   * Context to iterateDirectoryRecursive.
   */
  struct TraverseContext
  {
    /**
     * Time of last status report.
     */
    std::chrono::time_point<std::chrono::system_clock> lastReportTime =
      std::chrono::system_clock::now();
    /**
     * Total number of (regular) files visited in the iteration.
     */
    std::size_t numFilesVisited = 0;
    /**
     * Total number of directories visited in the iteration.
     */
    std::size_t numDirsVisited = 0;
  };
protected:
  using SourceTargetVector = std::vector<std::tuple<std::string, std::string>>;
  
  SourceTargetVector _sourceTargets;
  SourceTargetVector::iterator _sourceTargetsIter;
  
  unsigned long long _buildIdCounter;
  bool _linkStepReady;
private:
  /**
   * A build command.
   */
  struct BuildCommand
  {
    /**
     * Argument vector including the compiler command (first argument).
     */
    std::vector<std::string> arguments;
    /**
     * Source to target map.
     */
    std::map<std::string, std::string> sourceToTarget;
  };

  /**
   * Build action id -> BuildCommand map type.
   */
  using BuildActions = std::map<
      std::uint64_t,
      BuildCommand
    >;
  
  bool accept(const std::string& path_);
  bool initParse(const std::string& path_, Context& context_);
  bool getNextTask( Context& context_, ParserTask& task_);
  AsyncBuildAction parseCommand( const ParserTask& task_,
    std::size_t& numOfFails_);
  AsyncBuildAction parseFiles( const ParserTask& task_,
    std::size_t& numOfFails_);
  void loadTask( model::BuildActionPtr action_,
    const BuildActions::value_type& jsonAction_, ParserTask& task_);
  void createNewTask( const BuildActions::value_type& jsonAction_,
    ParserTask& task_);
  
  model::BuildActionPtr  addBuildAction(
    model::BuildAction::Type type_,
    uint64_t id,
    const std::string& label_ = "");

  model::BuildParameterPtr addBuildParameter(
    model::BuildActionPtr action_,
    const std::string& param_ );

  model::BuildSourcePtr addBuildSource(
    model::BuildActionPtr action_,
    const std::string& path_);

  model::BuildSourcePtr addBuildSource(
    model::BuildActionPtr action_,
    model::FilePtr file_);

  model::BuildTargetPtr addBuildTarget(
    model::BuildActionPtr action_,
    const std::string& path_);

  model::BuildTargetPtr addBuildTarget(
    model::BuildActionPtr action_,
    model::FilePtr file_);

  /**
   * Iterates over the content of the given directory (recursive) and calls the
   * given callback for it's content.
   *
   * It reports the progress
   *
   * @param context_ context (basically) for status reporting.
   * @param path_ a directory path
   * @param callbacks_ callback functions.
   * @return false if we should stop the iteration in the current branch.
   */
  static bool iterateDirectoryRecursive(
    TraverseContext& context_,
    const std::string& path_,
    std::vector<Traversal::DirIterCallback> callbacks_);
  
  /**
   * Project roots. Set by the constructor.
   */
  std::vector<std::string> _projPath;
  
  /**
   * Build action id -> BuildCommand map.
   */
  BuildActions _actions;
  
  /**
   * Iterator to the next build command. See getNextTask(ParserContext&,
   * ParserTask&) for details.
   */
  BuildActions::const_iterator _currentAction;
  
  /**
   * Set of build action IDs to skip.
   */
  std::unordered_set<std::uint64_t> _skippableActions;
  /**
   * If non-empty, only these actions will be parsed.
   */
  std::unordered_set<std::uint64_t> _debugActions;
  
  std::shared_ptr<cc::model::Workspace> _w;
};
  
}
}
#endif
