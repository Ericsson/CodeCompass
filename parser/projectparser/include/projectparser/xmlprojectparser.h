#ifndef PARSER_XMLPROJECTPARSER_H
#define PARSER_XMLPROJECTPARSER_H

#include <projectparser/xmlparser.h>

#include <parser/parser.h>
#include <parser/projectparser.h>
#include <parser/sourcemanager.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <tuple>

namespace cc
{

namespace model
{
  class Workspace;
}
namespace parser
{
  
/**
 * Traverses all the file of a project recursively and invokes the
 * specific parser for the different sources
 */
class XmlProjectParser : public ProjectParser
{
public:
  XmlProjectParser(
    std::shared_ptr<model::Workspace> w_,
    ParseProps& props_,
    SourceManager& srcMgr_) :
    ProjectParser(w_, props_, srcMgr_)
  {
  }

  virtual ~XmlProjectParser() {}

  virtual bool accept(const std::string& path_) override;
  
  virtual bool initParse(const std::string& path_, ParserContext& context_) override;
  
  virtual bool getNextTask(ParserContext& context_, ParserTask& task_) override;

  void skipActions(std::unordered_set<unsigned long> actions_)
  {
    _skippableActions = std::move(actions_);
  }

  void debugActions(std::unordered_set<unsigned long> actions_)
  {
    _debugActions = std::move(actions_);
  }

private:
  model::BuildTargetPtr findTarget(
    model::BuildSourcePtr source,
    const std::vector<model::BuildTargetPtr>& targets);

protected:
  XMLParser::BuildActions _actions;
  std::vector<XMLParser::BuildAction>::iterator _actionIterator;
  std::map<model::BuildAction::pktype, unsigned int> _actionStateInDb;
  std::vector<std::string> _opts;
  std::unordered_set<unsigned long> _skippableActions;
  std::unordered_set<unsigned long> _debugActions;
};

} // parser
} // cc

#endif //PARSER_XMLPROJECTPARSER_H
