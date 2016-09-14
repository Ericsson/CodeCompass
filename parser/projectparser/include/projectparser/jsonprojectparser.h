#ifndef __CC_JSON_PROJECT_PARSER__
#define __CC_JSON_PROJECT_PARSER__

#include <parser/parser.h>
#include <parser/projectparser.h>
#include <parser/sourcemanager.h>

#include <string>
#include <unordered_set>

#endif // __CC_JSON_PROJECT_PARSER__

namespace cc
{

namespace model
{
  class Workspace;
}

namespace parser
{

/**
 * Project parser implementation for parsing clang JSON compilation databases.
 */
class JSonProjectParser : public ProjectParser
{
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

public:
  /**
   * Construct a JSON project parser.
   *
   * @param w_ the workspace (for DB).
   * @param props_ parse properties.
   * @param srcMgr_ the source managers.
   * @param roots_ project roots for traversals.
   */
  JSonProjectParser(
    std::shared_ptr<model::Workspace> w_,
    ParseProps& props_,
    SourceManager& srcMgr_,
    const ProjectRoots& roots_);

  /**
   * The destructor. Does nothing.
   */
  ~JSonProjectParser();

  /**
   * Returns true json files.
   */
  virtual bool accept(const std::string& path_) override;

  virtual bool initParse(
    const std::string& path_,
    ParserContext& context_) override;

  virtual bool getNextTask(
    ParserContext& context_,
    ParserTask& task_) override;

  void skipActions(std::unordered_set<std::uint64_t> actions_)
  {
    _skippableActions = std::move(actions_);
  }

  void debugActions(std::unordered_set<std::uint64_t> actions_)
  {
    _debugActions = std::move(actions_);
  }

private:
  /**
   * Creates a task from a json command.
   *
   * @param jsonAction_ a build id, command pair.
   * @param task_ the result task.
   */
  void createNewTask(
    const BuildActions::value_type& jsonAction_,
    ParserTask& task_);

  /**
   * Loads a task from the database.
   *
   * @param action_ a build action (pevoiusly loaded from the database).
   * @param jsonAction_ a build id, command pair.
   * @param task_ the result task.
   */
  void loadTask(
    model::BuildActionPtr action_,
    const BuildActions::value_type& jsonAction_,
    ParserTask& task_);

private:
  /**
   * Project roots. Set by the constructor.
   */
  ProjectRoots _projRoots;
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
};

} // parser
} // cc
