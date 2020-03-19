#include <cmath>

#include "diagram.h"
#include <util/odbtransaction.h>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>

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

    //graph.setNodeAttribute(node, "label");
    decorateNode(graph_, node, competenceNodeDecoration);
  });
}

util::Graph::Node CompetenceDiagram::addNode(
  util::Graph& graph_,
  const language::AstNodeInfo& nodeInfo_)
{
  util::Graph::Node node
    = graph_.getOrCreateNode(nodeInfo_.id,
      addSubgraph(graph_, nodeInfo_.range.file));

  graph_.setNodeAttribute(node, "label", nodeInfo_.astNodeValue);

  return node;
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

std::string rateToColor(short rate)
{
  std::uint32_t red, green;

  red = rate < 50 ? 255 : std::round(256 - (rate - 50) * 5.12);
  green = rate > 50 ? 255 : std::round(rate * 5.12);

  std::stringstream ss;
  ss << "#";
  ss << std::hex << (red << 16 | green << 8 | 0);
  return ss.str();
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
}
}
}