#include <algorithm>
#include <regex>
#include <sstream>

#include <boost/filesystem.hpp>

#include <model/microserviceedge.h>
#include <model/microserviceedge-odb.hxx>
#include <model/helmtemplate.h>
#include <model/helmtemplate-odb.hxx>

#include <model/file.h>
#include <service/yamlservice.h>
#include <util/dbutil.h>

#include "yamlfilediagram.h"

namespace cc
{
namespace service
{
namespace language
{

namespace fs = boost::filesystem;

typedef odb::query<model::MicroserviceEdge> EdgeQuery;
typedef odb::result<model::MicroserviceEdge> EdgeResult;
typedef odb::query<model::Microservice> MicroserviceQuery;
typedef odb::result<model::Microservice> MicroserviceResult;
typedef odb::query<model::HelmTemplate> HelmTemplateQuery;
typedef odb::result<model::HelmTemplate> HelmTemplateResult;

YamlFileDiagram::YamlFileDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _yamlHandler(db_, datadir_, context_),
    _projectHandler(db_, datadir_, context_)
{
}

void YamlFileDiagram::getYamlFileInfo(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  std::string htmlContent = "";
    _transaction([&, this](){

      FilePathResult yamlPath = _db->query<model::File>(
    FilePathQuery::id == std::stoull(fileId_));

      YamlResult yamlInfo = _db->query<model::YamlFile>(
    YamlQuery::file == std::stoull(fileId_));

      YamlContentResult yamlContent = _db->query<model::YamlContent>(
    YamlContentQuery::file == std::stoull(fileId_));

      core::FileInfo fileInfo;
      _projectHandler.getFileInfo(fileInfo, fileId_);
      util::Graph::Node node = addNode(graph_, fileInfo);

      int numOfDataPairs = yamlContent.size();
      std::string type = typeToString(yamlInfo.begin()->type);
      std::string path = yamlPath.begin()->path;

      htmlContent +=  "<p><strong> FileType: </strong>" + type + "</p>";
      htmlContent += "<p><strong> FilePath: </strong>" + path + "</p>";
      htmlContent += "<p><strong> Key-data pairs: </strong>"
                     + std::to_string(numOfDataPairs) + "</p>";

      htmlContent += graphHtmlTag("p",
            graphHtmlTag("strong", "Main Keys:"));

      htmlContent += "<ul>";

      for (const model::YamlContent& yc : yamlContent)
      {
        htmlContent += graphHtmlTag("li", yc.key);
      }

      htmlContent += "</ul>";
      graph_.setNodeAttribute(node, "FileInfo", htmlContent, true);
  });
}


void YamlFileDiagram::getYamlFileDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  std::string thAttr
          = "style=\"background-color:lightGray; font-weight:bold; height:50px\"";

  std::string tdAttr = "style=\"height:30px\"";
  std::string table = "<table border='1' cellspacing='1' width='75%'>";

  _transaction([&, this](){
      typedef odb::result<model::YamlContent> YamlResult;
      typedef odb::query<model::YamlContent> YamlQuery;

      YamlResult yamlContent = _db->query<model::YamlContent>(
              YamlQuery::file == std::stoull(fileId_));

      core::FileInfo fileInfo;
      _projectHandler.getFileInfo(fileInfo, fileId_);
      util::Graph::Node node = addNode(graph_, fileInfo);

      table += graphHtmlTag("tr",
        graphHtmlTag("th", "Key", thAttr) +
        graphHtmlTag("th", "Value", thAttr));

      for (const model::YamlContent& yc : yamlContent)
      {
        table += graphHtmlTag("tr",
          graphHtmlTag("td", yc.key, tdAttr) +
          graphHtmlTag("td", yc.value, tdAttr));
      }
      table.append("</table>");

      graph_.setNodeAttribute(node, "content", table, true);
  });
}

void YamlFileDiagram::getMicroserviceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);
  util::Graph::Node currentNode = addNode(graph_, fileInfo);

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getMicroservices,
    this, std::placeholders::_1, std::placeholders::_2),
    microserviceNodeDecoration, {}, 1);
}

std::vector<util::Graph::Node> YamlFileDiagram::getMicroservices(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> microservices;

  _transaction([&, this] {
    for (const model::Microservice& service : _db->query<model::Microservice>())
    {
      microservices.push_back(addNode(graph_, service));
    }
  });

  return microservices;
}

/* ---- Dependent microservices ---- */

void YamlFileDiagram::getDependentServicesDiagram(
  util::Graph& graph_,
  const language::MicroserviceId& serviceId_)
{
  util::Graph::Node currentNode;

  _transaction([&, this]{
    MicroserviceResult res = _db->query<model::Microservice>(
      MicroserviceQuery::serviceId == std::stoull(serviceId_));

    currentNode = addNode(graph_, *res.begin());
  });

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getDependencies,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getRevDependencies,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});
}

std::vector<util::Graph::Node> YamlFileDiagram::getDependencies(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependentServices(graph_, node_);
}

std::vector<util::Graph::Node> YamlFileDiagram::getRevDependencies(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependentServices(graph_, node_, true);
}

std::vector<util::Graph::Node> YamlFileDiagram::getDependentServices(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> dependencies;
  std::multimap<model::MicroserviceId, std::string> serviceIds = getDependentServiceIds(graph_, node_, reverse_);

  for (const auto& serviceId : serviceIds)
  {
    _transaction([&, this]{
      MicroserviceResult res = _db->query<model::Microservice>(
        MicroserviceQuery::serviceId == serviceId.first);

      util::Graph::Node newNode = addNode(graph_, *res.begin());
      dependencies.push_back(newNode);
      util::Graph::Edge edge = graph_.createEdge(node_, newNode);
      decorateEdge(graph_, edge, {{"label", serviceId.second}});
    });
  }

  return dependencies;
}

std::multimap<model::MicroserviceId, std::string> YamlFileDiagram::getDependentServiceIds(
  util::Graph&,
  const util::Graph::Node& node_,
  bool reverse_) {
  std::multimap<model::MicroserviceId, std::string> dependencies;

  _transaction([&, this] {
    EdgeResult res = _db->query<model::MicroserviceEdge>(
      (reverse_
       ? EdgeQuery::to->serviceId
       : EdgeQuery::from->serviceId) == std::stoull(node_)
      && (EdgeQuery::type == "Service"));

    for (const model::MicroserviceEdge &edge: res) {
      model::MicroserviceId serviceId = reverse_ ? edge.from->serviceId : edge.to->serviceId;
      dependencies.insert({serviceId, edge.type});
    }
  });

  return dependencies;
}

/* ---- Generated config maps ---- */

void YamlFileDiagram::getConfigMapsDiagram(
  util::Graph& graph_,
  const language::MicroserviceId& serviceId_)
{
  util::Graph::Node currentNode;

  _transaction([&, this]{
    MicroserviceResult res = _db->query<model::Microservice>(
      MicroserviceQuery::serviceId == std::stoull(serviceId_));

    currentNode = addNode(graph_, *res.begin());
  });

  LOG(info) << "step 1";

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getConfigMaps,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getRevConfigMaps,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});
}

std::vector<util::Graph::Node> YamlFileDiagram::getConfigMaps(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  LOG(info) << "step 2";
  return getDependentConfigMaps(graph_, node_);
}

std::vector<util::Graph::Node> YamlFileDiagram::getRevConfigMaps(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  LOG(info) << "step 3";
  return getDependentConfigMaps(graph_, node_, true);
}

std::vector<util::Graph::Node> YamlFileDiagram::getDependentConfigMaps(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> dependencies;
  std::multimap<model::MicroserviceId, std::string> serviceIds = getDependentConfigMapIds(graph_, node_, reverse_);
  LOG(info) << "step 4";
  for (const auto& serviceId : serviceIds)
  {
    _transaction([&, this]{
      MicroserviceResult res = _db->query<model::Microservice>(
        MicroserviceQuery::serviceId == serviceId.first);

      util::Graph::Node newNode = addNode(graph_, *res.begin());
      dependencies.push_back(newNode);
      util::Graph::Edge edge = graph_.createEdge(node_, newNode);
      decorateEdge(graph_, edge, {{"label", serviceId.second}});
      LOG(info) << "step 5";
    });
  }

  return dependencies;
}

std::multimap<model::MicroserviceId, std::string> YamlFileDiagram::getDependentConfigMapIds(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::multimap<model::MicroserviceId, std::string> dependencies;

  _transaction([&, this]{
    EdgeResult res = _db->query<model::MicroserviceEdge>(
      (reverse_
       ? EdgeQuery::to->serviceId
       : EdgeQuery::from->serviceId) == std::stoull(node_)
      && (EdgeQuery::type == "ConfigMap"));

    for (const model::MicroserviceEdge& edge : res)
    {
      auto helm = _db->query_one<model::HelmTemplate>(
        HelmTemplateQuery::id == edge.connection->id);
      model::MicroserviceId serviceId = reverse_ ? edge.from->serviceId : edge.to->serviceId;
      dependencies.insert({serviceId, helm->name});
    }
  });

  return dependencies;
}

/* ---- Secrets ---- */

void YamlFileDiagram::getSecretsDiagram(
  util::Graph& graph_,
  const language::MicroserviceId& serviceId_)
{
  util::Graph::Node currentNode;

  _transaction([&, this]{
    MicroserviceResult res = _db->query<model::Microservice>(
      MicroserviceQuery::serviceId == std::stoull(serviceId_));

    currentNode = addNode(graph_, *res.begin());
  });

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getSecrets,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});

  util::bfsBuild(graph_, currentNode, std::bind(&YamlFileDiagram::getRevSecrets,
    this, std::placeholders::_1, std::placeholders::_2),
    {}, {});
}

std::vector<util::Graph::Node> YamlFileDiagram::getSecrets(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependentSecrets(graph_, node_);
}

std::vector<util::Graph::Node> YamlFileDiagram::getRevSecrets(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  return getDependentSecrets(graph_, node_, true);
}

std::vector<util::Graph::Node> YamlFileDiagram::getDependentSecrets(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::vector<util::Graph::Node> dependencies;
  std::multimap<model::MicroserviceId, std::string> secretIds = getDependentSecretIds(graph_, node_, reverse_);

  for (const auto& secretId : secretIds)
  {
    _transaction([&, this]{
      MicroserviceResult res = _db->query<model::Microservice>(
        MicroserviceQuery::serviceId == secretId.first);

      util::Graph::Node newNode = addNode(graph_, *res.begin());
      dependencies.push_back(newNode);
      util::Graph::Edge edge = graph_.createEdge(node_, newNode);
      decorateEdge(graph_, edge, {{"label", secretId.second}});
    });
  }

  return dependencies;
}

std::multimap<model::MicroserviceId, std::string> YamlFileDiagram::getDependentSecretIds(
  util::Graph&,
  const util::Graph::Node& node_,
  bool reverse_)
{
  std::multimap<model::MicroserviceId, std::string> dependencies;

  _transaction([&, this]{
    EdgeResult res = _db->query<model::MicroserviceEdge>(
      (reverse_
       ? EdgeQuery::to->serviceId
       : EdgeQuery::from->serviceId) == std::stoull(node_)
      && (EdgeQuery::type == "Secret"));

    for (const model::MicroserviceEdge& edge : res)
    {
      auto helm = _db->query_one<model::HelmTemplate>(
        HelmTemplateQuery::id == edge.connection->id);
      model::MicroserviceId serviceId = reverse_ ? edge.from->serviceId : edge.to->serviceId;
      dependencies.insert({serviceId, helm->name});
    }
  });

  return dependencies;
}

std::string YamlFileDiagram::graphHtmlTag(
  const std::string& tag_,
  const std::string& content_,
  const std::string& attr_)
{
  return std::string("<")
    .append(tag_)
    .append(" ")
    .append(attr_)
    .append(">")
    .append(content_)
    .append("</")
    .append(tag_)
    .append(">");
}

util::Graph::Node YamlFileDiagram::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  util::Graph::Node node_ = graph_.getOrCreateNode(fileInfo_.id);
  graph_.setNodeAttribute(node_, "label", getLastNParts(fileInfo_.path, 3));

  if (fileInfo_.type == model::File::DIRECTORY_TYPE)
  {
    decorateNode(graph_, node_, directoryNodeDecoration);
  }
  else if (fileInfo_.type == model::File::BINARY_TYPE)
  {
    decorateNode(graph_, node_, binaryFileNodeDecoration);
  }
  else
  {
    std::string ext = boost::filesystem::extension(fileInfo_.path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".yaml" || ext == ".yml")
      decorateNode(graph_, node_, sourceFileNodeDecoration);
  }

  return node_;
}

util::Graph::Node YamlFileDiagram::addNode(
  util::Graph& graph_,
  const model::Microservice& service_)
{
  util::Graph::Node node_ = graph_.getOrCreateNode(std::to_string(service_.serviceId));
  graph_.setNodeAttribute(node_, "label", service_.name);

  decorateNode(graph_, node_, microserviceNodeDecoration);

  return node_;
}

std::string YamlFileDiagram::getLastNParts(
        const std::string& path_,
        std::size_t n_)
{
  if (path_.empty() || n_ == 0)
    return "";

  std::size_t p;
  for (p = path_.rfind('/');
       --n_ > 0 && p - 1 < path_.size();
       p = path_.rfind('/', p - 1));

  return p > 0 && p < path_.size() ? "..." + path_.substr(p) : path_;
}

void YamlFileDiagram::decorateNode(
        util::Graph& graph_,
        const util::Graph::Node& node_,
        const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void YamlFileDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

const YamlFileDiagram::Decoration
  YamlFileDiagram::sourceFileNodeDecoration = {
  {"shape", "box"},
  {"style", "filled"},
  {"fillcolor", "#116db6"},
  {"fontcolor", "white"}
};

const YamlFileDiagram::Decoration
        YamlFileDiagram::directoryNodeDecoration = {
  {"shape", "folder"}
};

const YamlFileDiagram::Decoration
  YamlFileDiagram::binaryFileNodeDecoration = {
  {"shape", "box3d"},
  {"style", "filled"},
  {"fillcolor", "#f18a21"},
  {"fontcolor", "white"}
};

const YamlFileDiagram::Decoration YamlFileDiagram::microserviceNodeDecoration = {
  {"shape", "folder"},
  {"color", "blue"}
};

const YamlFileDiagram::Decoration YamlFileDiagram::dependsEdgeDecoration = {
  {"label", "depends on"}
};

}
}
}