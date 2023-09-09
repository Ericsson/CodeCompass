#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include <boost/filesystem.hpp>

#include "valueanalyzer.h"

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

std::unordered_set<model::MicroserviceEdgeId> ValueAnalyzer::_edgeCache;
std::vector<model::Microservice> ValueAnalyzer::_microserviceCache;
std::mutex ValueAnalyzer::_edgeCacheMutex;

ValueAnalyzer::ValueAnalyzer(
  ParserContext& ctx_,
  std::map<std::string, YAML::Node>& fileAstCache_,
  std::uint64_t templateIdCounter)
  : _templateCounter(templateIdCounter), _ctx(ctx_), _fileAstCache(fileAstCache_)
{
  std::lock_guard<std::mutex> cacheLock(_edgeCacheMutex);

  if (_edgeCache.empty())
  {
    util::OdbTransaction{_ctx.db}([this]
    {
      for (const model::MicroserviceEdge& edge : _ctx.db->query<model::MicroserviceEdge>())
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

ValueAnalyzer::~ValueAnalyzer()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    for (model::HelmTemplate& helmTemplate : _newTemplates)
      _ctx.db->persist(helmTemplate);

    util::persistAll(_newEdges, _ctx.db);
  });
}

void ValueAnalyzer::init()
{
  (util::OdbTransaction(_ctx.db))([this]{
    std::for_each(_fileAstCache.begin(), _fileAstCache.end(),
    [&, this](std::pair<std::string, YAML::Node> pair)
    {
      auto filePtr = _ctx.db->query_one<model::File>(odb::query<model::File>::path == pair.first);
      auto currentService = std::find_if(_microserviceCache.begin(),
        _microserviceCache.end(),
        [&](model::Microservice& service)
        {
          return pair.first.find(service.name) != std::string::npos;
        });

      if (currentService != _microserviceCache.end())
        visitKeyValuePairs(pair.second, *currentService, filePtr);
    });
  });
}

bool ValueAnalyzer::visitKeyValuePairs(
  YAML::Node& currentNode_,
  model::Microservice& service_,
  const model::FilePtr& file_)
{
  for (auto it = currentNode_.begin(); it != currentNode_.end(); ++it)
  {
    if (it->second.IsDefined() && !it->second.IsScalar())
      visitKeyValuePairs(it->second, service_, file_);
    else
    {
      std::string current(YAML::Dump(it->second));
      auto iter = std::find_if(_microserviceCache.begin(),
        _microserviceCache.end(),
        [&, this](const model::Microservice& other) {
          return current == other.name;
      });

      if (iter != _microserviceCache.end())
      {
        model::HelmTemplate helmTemplate;
        helmTemplate.name = "";
        helmTemplate.dependencyType = model::HelmTemplate::DependencyType::SERVICE;
        helmTemplate.depends = service_.serviceId;
        helmTemplate.kind = "Service";
        helmTemplate.file = file_->id;
        helmTemplate.id = createIdentifier(helmTemplate);
        addHelmTemplate(helmTemplate);

        addEdge(service_.serviceId, iter->serviceId, helmTemplate.id, "Service");
      }
    }
  }
}

void ValueAnalyzer::addHelmTemplate(
  model::HelmTemplate& helmTemplate_)
{
  auto it = std::find_if(_newTemplates.begin(), _newTemplates.end(),
    [&](auto& helm)
    {
     return helm.id == helmTemplate_.id;
    });

  if (it == _newTemplates.end())
    _newTemplates.push_back(helmTemplate_);
}

void ValueAnalyzer::addEdge(
  const model::MicroserviceId& from_,
  const model::MicroserviceId& to_,
  const model::HelmTemplateId& connect_,
  std::string type_)
{
  static std::mutex m;
  std::lock_guard<std::mutex> guard(m);

  model::MicroserviceEdgePtr edge = std::make_shared<model::MicroserviceEdge>();

  edge->from = std::make_shared<model::Microservice>();
  edge->from->serviceId = from_;
  edge->to = std::make_shared<model::Microservice>();
  edge->to->serviceId = to_;

  edge->connection = std::make_shared<model::HelmTemplate>();
  edge->connection->id = connect_;

  edge->type = std::move(type_);
  edge->helperId = ++_templateCounter;
  edge->id = model::createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _newEdges.push_back(edge);
  }
}

}
}