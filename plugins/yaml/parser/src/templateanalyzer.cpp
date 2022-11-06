#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include <model/helmtemplate.h>
#include <model/helmtemplate-odb.hxx>

#include "templateanalyzer.h"

namespace cc
{
namespace parser
{

std::unordered_set<model::MicroserviceEdgeId> TemplateAnalyzer::_edgeCache;
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

TemplateAnalyzer::~TemplateAnalyzer()
{
  (util::OdbTransaction(_ctx.db))([this]{
    for (model::HelmTemplate& helmTemplate : _newTemplates)
      _ctx.db->persist(helmTemplate);

    util::persistAll(_newEdges, _ctx.db);
  });
}

void TemplateAnalyzer::init()
{
  (util::OdbTransaction(_ctx.db))([this]{
    std::for_each(_fileAstCache.begin(), _fileAstCache.end(),
    [&, this](std::pair<std::string, YAML::Node> pair)
    {
      auto currentService = std::find_if(_microserviceCache.begin(),
        _microserviceCache.end(),
        [&](model::Microservice& service)
        {
          //auto filePtr = _ctx.db->query_one<model::File>(odb::query<model::File>::path == pair.first);
          //return service.file == filePtr->id;
          return pair.first.find(service.name) != std::string::npos;
        });

      visitKeyValuePairs(pair.first, pair.second, *currentService);
    });
  });
}

bool TemplateAnalyzer::visitKeyValuePairs(
  std::string path_,
  YAML::Node& currentNode_,
  model::Microservice& service_)
{
  typedef model::HelmTemplate::DependencyType DependencyType;

  if (!currentNode_["metadata"] ||
      !currentNode_["metadata"].IsMap() ||
      !currentNode_["kind"] ||
      !currentNode_["metadata"]["name"])
    return false;

  auto type = _dependencyPairs.find(YAML::Dump(currentNode_["kind"]));
  if (type == _dependencyPairs.end())
    return false;

  switch (type->second)
  {
    case DependencyType::SERVICE:
      processServiceDeps(path_, currentNode_, service_);
      break;
    case DependencyType::MOUNT:
      processMountDeps(path_, currentNode_, service_);
      break;
    case DependencyType::CERTIFICATE:
      processCertificateDeps(path_, currentNode_);
      break;
    case DependencyType::RESOURCE:
    case DependencyType::OTHER:
      break;
  }

  return true;
}

void TemplateAnalyzer::processServiceDeps(
  const std::string& path_,
  YAML::Node& currentFile_,
  model::Microservice& service_)
{
  /* --- Process Service templates --- */

  // Find MS in database.
  auto serviceIter = std::find_if(_microserviceCache.begin(), _microserviceCache.end(),
    [&](const model::Microservice& service)
    {
      return service.name == Dump(currentFile_["metadata"]["name"]);
    });

  // Persist template data to db.
  model::HelmTemplate helmTemplate;
  helmTemplate.dependencyType = model::HelmTemplate::DependencyType::SERVICE;

  auto filePtr = _ctx.db->query_one<model::File>(odb::query<model::File>::path == path_);
  helmTemplate.file = filePtr->id;
  helmTemplate.kind = Dump(currentFile_["kind"]);

  // If the MS is not present in the db,
  // it is an external / central MS,
  // and should be added to the db.
  if (serviceIter == _microserviceCache.end())
  {
    model::Microservice externalService;
    externalService.name = Dump(currentFile_["metadata"]["name"]);
    externalService.type = model::Microservice::ServiceType::EXTERNAL;
    externalService.file = filePtr->id;
    externalService.serviceId = createIdentifier(externalService);
    _ctx.db->persist(externalService);

    helmTemplate.depends = externalService.serviceId;
  }
  else
  {
    helmTemplate.depends = serviceIter->serviceId;
  }

  _newTemplates.push_back(helmTemplate);
  addEdge(service_.serviceId, helmTemplate.depends, "Service");
}

void TemplateAnalyzer::processMountDeps(
  const std::string& path_,
  YAML::Node& currentFile_,
  model::Microservice& service_)
{
  /* --- Processing ConfigMap templates --- */


  auto serviceIter = std::find_if(_microserviceCache.begin(), _microserviceCache.end(),
    [&](const model::Microservice& service)
    {
      return service.name == Dump(currentFile_["metadata"]["name"]);
    });

  // Self-dependencies should be omitted.
  if (path_.find(serviceIter->name) != std::string::npos)
    return;

  // Persist the template data into the database.
  model::HelmTemplate helmTemplate;
  helmTemplate.dependencyType = model::HelmTemplate::DependencyType::MOUNT;

  auto filePtr = _ctx.db->query_one<model::File>(odb::query<model::File>::path == path_);
  helmTemplate.file = filePtr->id;

  auto kindIter = currentFile_.as<std::map<std::string, YAML::Node>>().find("kind");
  helmTemplate.kind = Dump(kindIter->second);

  if (serviceIter == _microserviceCache.end())
  {
    // If the microservice to which the template belongs
    // is not present in the database,
    // process and persist it.
    model::Microservice externalService;
    externalService.name = Dump(currentFile_["metadata"]["name"]);
    externalService.type = model::Microservice::ServiceType::EXTERNAL;
    externalService.file = filePtr->id;
    externalService.serviceId = createIdentifier(externalService);
    _ctx.db->persist(externalService);

    helmTemplate.depends = externalService.serviceId;
  }
  else
  {
    helmTemplate.depends = serviceIter->serviceId;
  }

  _newTemplates.push_back(helmTemplate);
  addEdge(service_.serviceId, helmTemplate.depends, helmTemplate.kind);
}

void TemplateAnalyzer::processCertificateDeps(
  const std::string& path_,
  YAML::Node& currentFile_)
{

}

YAML::Node TemplateAnalyzer::findKey(
  const std::string& key_,
  YAML::Node& node_)
{
  switch (node_.Type())
  {
    case YAML::NodeType::Scalar:
      break;
    case YAML::NodeType::Null:
    case YAML::NodeType::Undefined:
      break;
    case YAML::NodeType::Sequence:
      for (auto elem : node_)
        if (elem.IsMap())
          return findKey(key_, elem);
      break;
    case YAML::NodeType::Map:
      if (node_[key_])
        return node_[key_];
      else
        for (auto iter = node_.begin(); iter != node_.end(); ++iter)
        {
          YAML::Node temp = findKey(key_, iter->second);
          if (temp.IsDefined())
            return temp;
        }
      break;
  }

  return YAML::Node(YAML::NodeType::Undefined);
}

void TemplateAnalyzer::addEdge(
  const model::MicroserviceId& from_,
  const model::MicroserviceId& to_,
  std::string type_)
{
  static std::mutex m;
  std::lock_guard<std::mutex> guard(m);

  model::MicroserviceEdgePtr edge = std::make_shared<model::MicroserviceEdge>();

  edge->from = std::make_shared<model::Microservice>();
  edge->from->serviceId = from_;
  edge->to = std::make_shared<model::Microservice>();
  edge->to->serviceId = to_;

  edge->type = std::move(type_);
  edge->id = model::createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _newEdges.push_back(edge);
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