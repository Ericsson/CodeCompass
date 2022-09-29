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

bool TemplateAnalyzer::visitKeyValuePairs(
  YAML::Node& currentNode_,
  model::Microservice& service_)
{
  for (auto it = currentNode_.begin(); it != currentNode_.end(); ++it)
  {
    if (YAML::Dump(it->first) == "kind")
    {
      model::HelmTemplate helmTemplate;
      helmTemplate.kind = YAML::Dump(it->second);

      auto pair = _dependencyPairs.find(YAML::Dump(it->second));
      if (pair != _dependencyPairs.end())
      {
        helmTemplate.dependencyType = pair->second;
      }
      else
      {
        helmTemplate.dependencyType = model::HelmTemplate::DependencyType::OTHER;
      }
    }
  }
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