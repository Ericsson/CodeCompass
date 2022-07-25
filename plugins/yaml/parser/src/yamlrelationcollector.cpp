#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include "yamlrelationcollector.h"

namespace cc
{
namespace parser
{

YamlRelationCollector::YamlRelationCollector(
  ParserContext& ctx_,
  std::vector<YAML::Node>& fileAstCache_)
  : _ctx(ctx_), _fileAstCache(fileAstCache_)
{
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

YamlRelationCollector::~YamlRelationCollector()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    util::persistAll(_newEdges, _ctx.db);
  });
}

bool YamlRelationCollector::visitKeyValuePairs(
  YAML::Node& currentNode_,
  model::Microservice& service_)
{
  for (auto it = currentNode_.begin(); it != currentNode_.end(); ++it)
  {
    if (!it->second.IsScalar())
      visitKeyValuePairs(it->second, service_);
    else
    {
      std::string current(YAML::Dump(it->second));
      auto iter = std::find_if(_microserviceCache.begin(),
    _microserviceCache.end(),
    [&, this](const model::Microservice other) {
       return current == other.name;
      });

      if (iter != _microserviceCache.end())
        addEdge(service_.id, iter->id, YAML::Dump(it->first));
    }
  }
}

void YamlRelationCollector::addEdge(
  const model::MicroserviceId& from_,
  const model::MicroserviceId& to_,
  std::string type_)
{
  static std::mutex m;
  std::lock_guard<std::mutex> guard(m);

  model::YamlEdgePtr edge = std::make_shared<model::YamlEdge>();

  edge->from = std::make_shared<model::Microservice>();
  edge->from->id = from_;
  edge->to = std::make_shared<model::Microservice>();
  edge->to->id = to_;

  edge->type = std::move(type_);
  edge->id = model::createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _newEdges.push_back(edge);
  }
}

}
}