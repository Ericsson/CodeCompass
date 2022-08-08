#ifndef CC_PARSER_YAMLRELATIONCOLLECTOR_H
#define CC_PARSER_YAMLRELATIONCOLLECTOR_H

#include "yaml-cpp/yaml.h"

#include "model/file.h"

#include <model/microservice.h>
#include <model/microservice-odb.hxx>
#include <model/yamledge.h>
#include <model/yamledge-odb.hxx>

#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

class YamlRelationCollector
{
public:
  YamlRelationCollector(
    ParserContext& ctx_,
    std::map<std::string, YAML::Node>& fileAstCache_);

  void init();

  ~YamlRelationCollector();

private:
  void addEdge(
    const model::MicroserviceId& from_,
    const model::MicroserviceId& to_,
    std::string type_);

  bool visitKeyValuePairs(
    YAML::Node& currentFile_,
    model::Microservice& service_);

  static std::unordered_set<model::YamlEdgeId> _edgeCache;
  std::vector<model::YamlEdgePtr> _newEdges;

  static std::vector<model::Microservice> _microserviceCache;
  model::Microservice _currentService;

  static std::mutex _edgeCacheMutex;

  //YAML::Node& _loadedFile;
  ParserContext& _ctx;
  std::map<std::string, YAML::Node>& _fileAstCache;
};

}
}

#endif // CC_PARSER_YAMLRELATIONCOLLECTOR_H
