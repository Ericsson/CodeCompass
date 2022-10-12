#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include <model/helmtemplate.h>
#include <model/helmtemplate-odb.hxx>

#include "templateanalyzer.h"

namespace cc
{
namespace parser
{

std::unordered_set<model::YamlEdgeId> TemplateAnalyzer::_edgeCache;
std::vector<model::Microservice> TemplateAnalyzer::_microserviceCache;
std::mutex TemplateAnalyzer::_edgeCacheMutex;

TemplateAnalyzer::TemplateAnalyzer(
  cc::parser::ParserContext& ctx_,
  std::map<std::string, YAML::Node>& fileAstCache_)
  : _ctx(ctx_), _fileAstCache(fileAstCache_)
{
  fillDependencyPairsMap();

  std::lock_guard<std::mutex> cacheLock(_edgeCacheMutex);
  if (_edgeCache.empty())
  {
    util::OdbTransaction{_ctx.db}([this]
    {
      for (const model::YamlEdge& edge : _ctx.db->query<model::YamlEdge>())
      {
        _edgeCache.insert(edge.id);
      }
    });
  }

  if (_microserviceCache.empty())
  {
    util::OdbTransaction{_ctx.db}([this]
    {
      for (const model::Microservice& service : _ctx.db->query<model::Microservice>())
      {
        _microserviceCache.push_back(service);
      }
    });
  }
}

TemplateAnalyzer::~TemplateAnalyzer()
{
  (util::OdbTransaction(_ctx.db))([this]{
    for (model::HelmTemplate& helmTemplate : _newTemplates)
      _ctx.db->persist(helmTemplate);
  });
}

bool TemplateAnalyzer::visitKeyValuePairs(
  YAML::Node& currentNode_,
  model::Microservice& service_)
{
  typedef model::HelmTemplate::DependencyType DependencyType;
  for (auto it = currentNode_.begin(); it != currentNode_.end(); ++it)
  {
    if (YAML::Dump(it->first) == "kind")
    {
      auto type = _dependencyPairs.find(YAML::Dump(it->second));
      if (type == _dependencyPairs.end())
        break;

      switch (type->second)
      {
        case DependencyType::SERVICE:
          processServiceDeps(currentNode_);
          break;
        case DependencyType::MOUNT:
          processMountDeps(currentNode_);
          break;
        case DependencyType::CERTIFICATE:
          processCertificateDeps(currentNode_);
          break;
      }
    }
  }
}

void TemplateAnalyzer::processServiceDeps(YAML::Node& currentFile_)
{
  auto metadataIter = std::find_if(currentFile_.begin(), currentFile_.end(),
    [](const auto& rootNode)
    {
      return Dump(rootNode.first) == "metadata";
    });

  if (metadataIter == currentFile_.end() || !metadataIter->second.IsMap())
    return;

  auto nameIter = std::find_if(metadataIter->begin(), metadataIter->end(),
    [](const auto& pair)
    {
     return Dump(pair.first) == "name";
    });

  if (nameIter == metadataIter->end())
    return;

  auto serviceIter = std::find_if(_microserviceCache.begin(), _microserviceCache.end(),
    [&](const model::Microservice& service)
    {
      return service.name == Dump(nameIter->second);
    });

  model::HelmTemplate helmTemplate;
  helmTemplate.dependencyType = model::HelmTemplate::DependencyType::SERVICE;

  if (serviceIter == _microserviceCache.end())
  {
    model::Microservice externalService;
    externalService.name = Dump(nameIter->second);
    externalService.type = model::Microservice::ServiceType::EXTERNAL;
    createIdentifier(externalService);
    _ctx.db->persist(externalService);

    helmTemplate.depends = externalService.serviceId;
  }
  else
  {
    helmTemplate.depends = serviceIter->serviceId;
  }

  _newTemplates.push_back(helmTemplate);
}

void processMountDeps(YAML::Node& currentFile_)
{

}

void processCertificateDeps(YAML::Node& currentFile_)
{

}

void TemplateAnalyzer::fillDependencyPairsMap()
{
  _dependencyPairs.insert({"Service", model::HelmTemplate::DependencyType::SERVICE});
  _dependencyPairs.insert({"ConfigMap", model::HelmTemplate::DependencyType::MOUNT});
  _dependencyPairs.insert({"Secret", model::HelmTemplate::DependencyType::MOUNT});
  _dependencyPairs.insert({"Certificate", model::HelmTemplate::DependencyType::CERTIFICATE});
  _dependencyPairs.insert({"VolumeClaim", model::HelmTemplate::DependencyType::RESOURCE});
}

}
}