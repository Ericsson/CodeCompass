#ifndef PARSER_GENERALPROJECTPARSER_H
#define PARSER_GENERALPROJECTPARSER_H

#include <string>
#include <vector>
#include <functional>

// #include "semantics/semantic.h"
// #include "semantics/workspace.h"
#include <parser/parser.h>
#include <parser/projectparser.h>
#include <parser/sourcemanager.h>

#include <model/file.h>

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
class GeneralProjectParser : public ProjectParser
{
public:
  GeneralProjectParser(
    std::shared_ptr<model::Workspace> w_,
    ParseProps& props_,
    SourceManager& srcMgr_);

  virtual ~GeneralProjectParser() {}
  
  virtual bool accept(const std::string& path_) override;
  
  virtual bool initParse(const std::string& path_, ParserContext& context_) override;
  
  virtual bool getNextTask(ParserContext& context_, ParserTask& task_) override;

protected:
  using SourceTargetVector = std::vector<std::tuple<std::string, std::string>>;
  
  std::vector<std::string> _opts;
  SourceTargetVector _sourceTargets;
  SourceTargetVector::iterator _sourceTargetsIter;
  unsigned long long _buildIdCounter;
  bool _linkStepReady;
};

} // parser
} // cc

#endif //PARSER_GENERALPROJECTPARSER_H
