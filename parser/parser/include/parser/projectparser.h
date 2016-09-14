#ifndef PARSER_PROJECTPARSER_H
#define PARSER_PROJECTPARSER_H

#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <tuple>
#include <map>
#include <chrono>

#include <parser/commondefs.h>
#include <parser/sourcemanager.h>
#include <parser/traversal.h>
#include <model/buildaction.h>
#include <model/buildparameter.h>
#include <model/buildsource.h>
#include <model/buildtarget.h>
#include <model/file.h>
#include <util/odbtransaction.h>

namespace cc
{

namespace model
{
  class Workspace;
}

namespace parser
{

/**
 * A result future with the source's path.
 */
struct AsyncBuildResult
{
  /**
   * A parse result returned by a parser.
   */
  std::future<ParseResult> pres;
  /**
   * The absolute path for the parsed source file.
   */
  std::string sourcePath;
};

/**
 * Alias for a Result container.
 */
using AsyncBuildResults = std::vector<AsyncBuildResult>;

/**
 * An action with parse results (an action can contain multiple source files).
 */
struct AsyncBuildAction
{
  /**
   * Build action.
   */
  model::BuildActionPtr action;
  /**
   * Results for the sources under the action.
   */
  AsyncBuildResults parseResults;
};

/**
 * Abstract base class of all project parser. This class implements a template
 * method pattern (see parse() for details).
 */
class ProjectParser
{
public:
  /**
   * Parser progress callback. The first parameter is the total number of
   * actions, the second is the current action.
   */
  using ParserCallback = std::function<void(int,int)>;
  /**
   * Alias for traversal vector.
   */
  using Traversals = std::vector<std::shared_ptr<Traversal>>;
  /**
   * Represents a parser task.
   */
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
   * Map between root labels and directory paths.
   */
  using ProjectRoots = std::map<std::string, std::string>;
  /**
   * Parser context.
   */
  struct ParserContext
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
    /**
     * Project root map.
     */
    ProjectRoots roots;
  };

public:
  /**
   * Creates a ProjectParser.
   *
   * @param w_ a workspace.
   * @param props_ parer properties.
   * @param srcMgr_ a source manager.
   */
  ProjectParser(
    std::shared_ptr<model::Workspace> w_,
    ParseProps& props_,
    SourceManager& srcMgr_);

  virtual ~ProjectParser() {}

  /**
   * Returns true, when the type of a project fits to the current parser. E.g.:
   * if file is an xml file, then the xml project parser returns with true.
   *
   * @param path_ path to the project file.
   * @return true if
   */
  virtual bool accept(const std::string& path_) = 0;
  /**
   * Parse the given project. This is the template method.
   *
   * The following methods called (in order):
   *  1, initParse() [TR]
   *  2, getNextTask() (until it returns true). [TR]
   *
   * Note: All of the methods above that marked with [TR] called in a (separate)
   * transaction.
   *
   * @param path_ a path to the project descriptor / directory.
   * @param callback_ parser callback for tracking progress.
   */
  void parse(const std::string& path_, ParserCallback callback_);

private:
  AsyncBuildAction parseFiles(const ParserTask& task_, std::size_t& numOfFails_);

  AsyncBuildAction parseCommand(const ParserTask& task_, std::size_t& numOfFails_);

protected:
  /**
   * Inits the given parser context.
   *
   * @param path_ path to the project entity.
   * @param context_ parser context.
   * @return return true on success.
   */
  virtual bool initParse(const std::string& path_, ParserContext& context_) = 0;
  /**
    * Provides the next task.
    *
    * @param context_ parser context.
    * @param task_ the "real" return value.
    * @return false if no more task.
    */
  virtual bool getNextTask(ParserContext& context_, ParserTask& task_) = 0;

protected:
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

  std::shared_ptr<model::Workspace> _w;
  ParseProps _props;
  SourceManager& _srcMgr;

private:
  /**
   * Calls the suitable traverses for the files in the roots. It also handle
   * the nested roots (so all file traversed once only).
   *
   * @param projectRoots_ project roots.
   * @param projectOptions_ project options.
   * @param srcMgr_ source manager.
   */
  void traverseRoots(
    const std::map<std::string, std::string>& projectRoots_,
    const std::map<std::string, std::string>& projectOptions_,
    SourceManager& srcMgr_);

  /**
   * Iterate through the files in SourceManager and add statistics
   * to the Statistics table
   */
  void addFileStatistics();

  /**
   * Static stuff;
   */
public:
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
   * Creates a ParseProps.
   *
   * @param worspace_ a workspace (database).
   * @return a ParseProps.
   */
  static ParseProps createParseProperties(
    std::shared_ptr<model::Workspace> worspace_);
  /**
   * Adds a traversal parser to the project parsers.
   *
   * @param traversal_ a traversal.
   */
  static void registerTraversal(std::shared_ptr<Traversal> traversal_);
  /**
   * Removes a traversal parser form the project parsers.
   *
   * @param traversal_ a traversal.
   */
  static void deregisterTraversal(std::shared_ptr<Traversal> traversal_);

  /**
   * Registered traversals.
   */
  static Traversals _traversals;
};

} //parser
} //cc

#endif //PARSER_PROJECTPARSER_H
