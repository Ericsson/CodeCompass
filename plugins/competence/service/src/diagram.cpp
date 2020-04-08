#include <cmath>

#include "diagram.h"
#include <util/odbtransaction.h>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <util/legendbuilder.h>

namespace cc
{
namespace service
{
namespace competence
{

CompetenceDiagram::CompetenceDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _compHandler(db_, datadir_, context_),
      _projectHandler(db_, datadir_, context_),
      _transaction(db_)
{
}

void CompetenceDiagram::getDirectoryCompetenceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Node currentNode = graph_.getOrCreateNode(fileInfo.id);
  graph_.setNodeAttribute(currentNode, "label", fileInfo.name);
  decorateNode(graph_, currentNode, centerNodeDecoration);

  std::set<util::Graph::Node> subdirs = util::bfsBuild(graph_, currentNode,
    std::bind(&CompetenceDiagram::getSubDirs, this, std::placeholders::_1,
    std::placeholders::_2), {}, subdirEdgeDecoration);

  subdirs.insert(currentNode);

  for (const util::Graph::Node& subdir : subdirs)
  {
    for (const std::pair<util::Graph::Node, short> node : getFileCompetenceRates(graph_, subdir))
    {
      util::Graph::Edge edge = graph_.createEdge(subdir, node.first);
      decorateEdge(graph_, edge, containsEdgeDecoration);

      std::string color = rateToColor(node.second);

      Decoration competenceNodeDecoration = {
        {"shape", "box"},
        {"style", "filled"},
        {"fillcolor", color}
      };
      decorateNode(graph_, node.first, competenceNodeDecoration);
    }
  }
}

void CompetenceDiagram::getFileCompetenceDiagram(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  graph_.setAttribute("rankdir", "LR");

  util::Graph::Node node
    = graph_.getOrCreateNode(fileId_);

  _transaction([&, this]()
  {
    auto comp = _db->query<model::FileComprehension>(
      odb::query<model::FileComprehension>::file == std::stoull(fileId_));

    std::string color = "#ffffff";

    if (!comp.empty())
      color = rateToColor(comp.begin()->userRatio);

    Decoration competenceNodeDecoration = {
      {"shape", "box"},
      {"style", "filled"},
      {"fillcolor", color}
    };

    model::FilePtr file = _db->query_one<model::File>(
      odb::query<cc::model::File>::id == std::stoull(fileId_));
    graph_.setNodeAttribute(node, "label", file->path);

    decorateNode(graph_, node, competenceNodeDecoration);
  });
}

std::string CompetenceDiagram::getFileCompetenceDiagramLegend()
{
  util::LegendBuilder builder("File-level competence diagram");

  return builder.getOutput();
}

std::map<util::Graph::Node, short> CompetenceDiagram::getFileCompetenceRates(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::map<core::FileId, short> comprehension;
  _transaction([&, this]
  {
    auto contained = _db->query<model::File>(
      odb::query<model::File>::parent == std::stoull(node_) &&
      odb::query<model::File>::type != model::File::DIRECTORY_TYPE
    );

    for (const model::File& file : contained)
    {
      auto comp = _db->query<model::FileComprehension>(
        odb::query<model::FileComprehension>::file == file.id
        );

      for (const model::FileComprehension& c : comp)
      {
        comprehension.insert(std::make_pair(std::to_string(c.file), c.userRatio));
      }
    }
  });

  std::map<util::Graph::Node, short> annotated;

  for (const auto& pair : comprehension)
  {
    core::FileInfo fileInfo;
    _projectHandler.getFileInfo(fileInfo, pair.first);

    util::Graph::Node node = graph_.getOrCreateNode(fileInfo.id);
    graph_.setNodeAttribute(node, "label", fileInfo.name);
    annotated.insert(std::make_pair(node, pair.second));
  }

  return annotated;
}


util::Graph::Node CompetenceDiagram::addNode(
  util::Graph& graph_,
  const core::FileInfo& fileInfo_)
{
  /*util::Graph::Node node
    = graph_.getOrCreateNode(nodeInfo_.id,
      addSubgraph(graph_, nodeInfo_.range.file));

  graph_.setNodeAttribute(node, "label", nodeInfo_.astNodeValue);

  return node;*/
}

util::Graph::Subgraph CompetenceDiagram::addSubgraph(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  auto it = _subgraphs.find(fileId_);

  if (it != _subgraphs.end())
    return it->second;

  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Subgraph subgraph
    = graph_.getOrCreateSubgraph("cluster_" + fileInfo.path);

  graph_.setSubgraphAttribute(subgraph, "id", fileInfo.id);
  graph_.setSubgraphAttribute(subgraph, "label", fileInfo.path);

  _subgraphs.insert(it, std::make_pair(fileInfo.path, subgraph));

  return subgraph;
}

std::string CompetenceDiagram::rateToColor(short rate)
{
  int red, green;

  red = rate < 50 ? 255 : std::round(256 - (rate - 50) * 5.12);
  green = rate > 50 ? 255 : std::round(rate * 5.12);

  if (green == 256)
    --green;

  if (red == 256)
    --red;

  std::stringstream ss;
  ss << "#";
  if (red != 0)
  {
    ss << std::hex << (red << 16 | green << 8 | 0);
  }
  else
  {
    ss << std::hex << 0 << 0 << (green << 8 | 0);
  }

  return ss.str();
}

std::vector<util::Graph::Node> CompetenceDiagram::getSubDirs(
  util::Graph& graph_,
  const util::Graph::Node& node_)
{
  std::vector<util::Graph::Node> usages;

  _transaction([&, this]
   {
     odb::result<model::File> sub = _db->query<model::File>(
       odb::query<model::File>::parent == std::stoull(node_) &&
       odb::query<model::File>::type == model::File::DIRECTORY_TYPE);

     for (const model::File &subdir : sub)
     {
       core::FileInfo fileInfo;
       _projectHandler.getFileInfo(fileInfo, std::to_string(subdir.id));

       util::Graph::Node node = graph_.getOrCreateNode(fileInfo.id);
       decorateNode(graph_, node, directoryNodeDecoration);
       graph_.setNodeAttribute(node, "label", fileInfo.name);
       usages.push_back(node);
     }
   });

  return usages;
}

void CompetenceDiagram::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void CompetenceDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

const CompetenceDiagram::Decoration CompetenceDiagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::directoryNodeDecoration = {
  {"shape", "folder"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::containsEdgeDecoration = {
  {"label", "contains"},
  {"color", "blue"}
};

const CompetenceDiagram::Decoration CompetenceDiagram::subdirEdgeDecoration = {
  {"label", "subdir"},
  {"color", "green"}
};
}
}
}