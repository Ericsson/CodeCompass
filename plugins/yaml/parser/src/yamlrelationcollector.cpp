#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include "yamlrelationcollector.h"

namespace cc
{
namespace parser
{

YamlRelationCollector::YamlRelationCollector(
  ParserContext &ctx_)
  : _ctx(ctx_)
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
}

YamlRelationCollector::~YamlRelationCollector()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    util::persistAll(_newEdges, _ctx.db);
  });
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

  edge->type = type_;
  edge->id = model::createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _newEdges.push_back(edge);
  }
}

}
}