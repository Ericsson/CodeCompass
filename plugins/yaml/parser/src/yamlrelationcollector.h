#ifndef CC_PARSER_YAMLRELATIONCOLLECTOR_H
#define CC_PARSER_YAMLRELATIONCOLLECTOR_H

#include "yaml-cpp/yaml.h"

#include "model/file.h"

#include <model/microservice.h>
#include <model/microservice-odb.hxx>
#include <model/microserviceedge.h>
#include <model/microserviceedge-odb.hxx>
#include <model/helmtemplate.h>
#include <model/helmtemplate-odb.hxx>

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
    std::map<std::string, YAML::Node>& fileAstCache_,
    uint64_t templateIdCounter);

  void init();

  ~YamlRelationCollector();

private:
  YAML::Node findValue(
    std::string value_,
    YAML::Node& currentFile_);

  void addEdge(
    const model::MicroserviceId& from_,
    const model::MicroserviceId& to_,
    const model::HelmTemplateId& connect_,
    std::string type_);

  bool visitKeyValuePairs(
    YAML::Node& currentNode_,
    model::Microservice& service_,
    const model::FilePtr& file_);

  void addHelmTemplate(
    model::HelmTemplate& helmTemplate_);

  static std::unordered_set<model::MicroserviceEdgeId> _edgeCache;
  std::vector<model::MicroserviceEdgePtr> _newEdges;
  std::vector<model::HelmTemplate> _newTemplates;
  uint64_t _templateCounter;

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
