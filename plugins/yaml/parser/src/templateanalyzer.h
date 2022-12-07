#ifndef CC_PARSER_TEMPLATEANALYZER_H
#define CC_PARSER_TEMPLATEANALYZER_H

#include "yaml-cpp/yaml.h"

#include "model/file.h"

#include <model/microservice.h>
#include <model/microservice-odb.hxx>
#include <model/microserviceedge.h>
#include <model/microserviceedge-odb.hxx>
#include <model/msresource.h>
#include <model/msresource-odb.hxx>
#include <model/helmtemplate.h>
#include <model/helmtemplate-odb.hxx>

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

  ~TemplateAnalyzer();

  void init();
  uint64_t getTemplateCounter() { return templateCounter; }

private:
  bool visitKeyValuePairs(
    std::string path_,
    YAML::Node& currentFile_,
    model::Microservice& service_);

  /**
   *
   * @param path_ The currently processed file path.
   * @param currentFile_ The currently processed file as a YAML node.
   * @param service_ The microservice in which the file is defined.
   */
  void processServiceDeps(
    const std::string& path_,
    YAML::Node& currentFile_,
    model::Microservice& service_);

  /**
   *
   * @param path_ The currently processed file path.
   * @param currentFile_ The currently processed file as a YAML node.
   * @param service_ The microservice in which the file is defined.
   */
  void processMountDeps(
    const std::string& path_,
    YAML::Node& currentFile_,
    model::Microservice& service_);

  /**
   *
   * @param path_ The currently processed file path.
   * @param currentFile_ The currently processed file as a YAML node.
   * @param service_ The microservice in which the file is defined.
   */
  void processCertificateDeps(
    const std::string& path_,
    YAML::Node& currentFile_,
    model::Microservice& service_);

  /**
   * Collect and store the various resources that a
   * cluster uses: CPU, memory, storage.
   * @param path_ The currently processed file path.
   * @param currentFile_ The currently processed file as a YAML node.
   * @param service_ The microservice in which the file is defined.
   */
  void processResources(
    const std::string& path_,
    YAML::Node& currentFile_,
    model::Microservice& service_);

  void addHelmTemplate(model::HelmTemplate& helmTemplate_);

  void addEdge(
    const model::MicroserviceId& from_,
    const model::MicroserviceId& to_,
    const model::HelmTemplateId& connect_,
    std::string type_);

  void fillDependencyPairsMap();
  void fillResourceTypePairsMap();

  std::pair<float, std::string> convertUnit(
    std::string amount_,
    model::MSResource::ResourceType type_);

  YAML::Node findKey(
    const std::string& key_,
    YAML::Node& currentFile_);

  std::map<std::string, model::HelmTemplate::DependencyType> _dependencyPairs;
  std::map<std::string, model::MSResource::ResourceType> _msResourcePairs;

  static std::unordered_set<model::MicroserviceEdgeId> _edgeCache;
  std::vector<model::MicroserviceEdgePtr> _newEdges;
  std::vector<model::HelmTemplate> _newTemplates;
  uint64_t templateCounter;

  static std::vector<model::Microservice> _microserviceCache;
  model::Microservice _currentService;

  std::vector<model::MSResource> _msResources;

  static std::mutex _edgeCacheMutex;

  ParserContext& _ctx;
  std::map<std::string, YAML::Node>& _fileAstCache;
};
}
}


#endif // CC_PARSER_TEMPLATEANALYZER_H
