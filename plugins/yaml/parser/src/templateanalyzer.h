#ifndef CC_PARSER_TEMPLATEANALYZER_H
#define CC_PARSER_TEMPLATEANALYZER_H

#include "yaml-cpp/yaml.h"

#include "model/file.h"

#include <model/helmtemplate.h>
#include <model/microservice.h>
#include <model/microservice-odb.hxx>
#include <model/yamledge.h>
#include <model/yamledge-odb.hxx>

#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

class TemplateAnalyzer
{
public:
  TemplateAnalyzer(
    ParserContext& ctx_,
    std::map<std::string, YAML::Node>& fileAstCache_);

private:
  bool visitKeyValuePairs(
    YAML::Node& currentFile_,
    model::Microservice& service_);

  void fillDependencyPairsMap();

  std::map<std::string, model::HelmTemplate::DependencyType> _dependencyPairs;

  static std::unordered_set<model::YamlEdgeId> _edgeCache;
  std::vector<model::YamlEdgePtr> _newEdges;

  static std::vector<model::Microservice> _microserviceCache;
  model::Microservice _currentService;

  static std::mutex _edgeCacheMutex;

  ParserContext& _ctx;
  std::map<std::string, YAML::Node>& _fileAstCache;
};
}
}


#endif // CC_PARSER_TEMPLATEANALYZER_H
