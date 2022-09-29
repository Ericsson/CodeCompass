#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include <boost/filesystem.hpp>

#include "yamlrelationcollector.h"

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

std::unordered_set<model::YamlEdgeId> YamlRelationCollector::_edgeCache;
std::vector<model::Microservice> YamlRelationCollector::_microserviceCache;
std::mutex YamlRelationCollector::_edgeCacheMutex;

YamlRelationCollector::YamlRelationCollector(
  ParserContext& ctx_,
  std::map<std::string, YAML::Node>& fileAstCache_)
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

void YamlRelationCollector::init()
{
  (util::OdbTransaction(_ctx.db))([this]{
    std::for_each(_fileAstCache.begin(), _fileAstCache.end(),
    [&, this](std::pair<std::string, YAML::Node> pair)
    {

      auto currentService = std::find_if(_microserviceCache.begin(),
      _microserviceCache.end(),
      [&](model::Microservice& service)
      {
        auto filePtr = _ctx.db->query_one<model::File>(odb::query<model::File>::path == pair.first);
        return service.file == filePtr->id;
      });
      visitKeyValuePairs(pair.second, *currentService);
    });
  });
}

bool YamlRelationCollector::visitKeyValuePairs(
  YAML::Node& currentNode_,
  model::Microservice& service_)
{
  for (auto it = currentNode_.begin(); it != currentNode_.end(); ++it)
  {
    if (it->second.IsDefined() && !it->second.IsScalar())
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

void YamlRelationCollector::processHelmTemplate(
  model::FilePtr file_,
  YAML::Node& loadedFile)
{

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
    LOG(warning) << "new edge added";
  }
}

}
}