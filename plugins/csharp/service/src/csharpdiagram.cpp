#include <boost/algorithm/string.hpp>

#include <service/csharpservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/legendbuilder.h>
#include <util/util.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include "csharpdiagram.h"

namespace
{

/**
 * Removes the annotation lines and the accessibility modifiers from the
 * signature of a node, and cuts the signature before the body.
 */
std::string toSignature(std::string str_)
{
  static const std::vector<std::string> accessibilityTags = {
    "protected internal", "private protected",
    "public", "private", "protected", "internal"};

  std::stringstream ss(str_), result;
  std::string line;

  while (std::getline(ss, line))
    if (line.find('[') != 0)
      result << line << '\n';

  str_ = result.str();
  boost::trim(str_);

  for (const auto& tag : accessibilityTags)
  {
    std::size_t pos = str_.find(tag);
    if (pos != std::string::npos)
      str_.erase(pos, tag.length() + 1);
  }

  return str_.substr(0, str_.find('{'));
}

}

namespace cc
{
namespace service
{
namespace language
{

CsharpDiagram::CsharpDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _projectHandler(db_, datadir_, context_)
{
}

void CsharpDiagram::getFunctionCallDiagram(
  util::Graph& graph_,
  const AstNodeInfo& centerNodeInfo_,
  const std::vector<AstNodeInfo>& calleeNodeInfos_,
  const std::vector<AstNodeInfo>& callerNodeInfos_)
{
  graph_.setAttribute("rankdir", "LR");

  AstNodeInfo centerInfo = centerNodeInfo_;
  centerInfo.astNodeValue = toSignature(centerInfo.astNodeValue);
  util::Graph::Node centerNode = addNode(graph_, centerInfo);
  decorateNode(graph_, centerNode, centerNodeDecoration);

  for (const auto& calleeNodeInfo : calleeNodeInfos_)
  {
    AstNodeInfo calleeInfo = calleeNodeInfo;
    calleeInfo.astNodeValue = toSignature(calleeInfo.astNodeValue);
    util::Graph::Node calleeNode = addNode(graph_, calleeInfo);
    decorateNode(graph_, calleeNode, calleeNodeDecoration);

    if (!graph_.hasEdge(centerNode, calleeNode))
    {
      util::Graph::Edge edge = graph_.createEdge(centerNode, calleeNode);
      decorateEdge(graph_, edge, calleeEdgeDecoration);
    }
  }

  for (const auto& callerNodeInfo : callerNodeInfos_)
  {
    AstNodeInfo callerInfo = callerNodeInfo;
    callerInfo.astNodeValue = toSignature(callerInfo.astNodeValue);
    util::Graph::Node callerNode = addNode(graph_, callerInfo);
    decorateNode(graph_, callerNode, callerNodeDecoration);

    if (!graph_.hasEdge(callerNode, centerNode))
    {
      util::Graph::Edge edge = graph_.createEdge(callerNode, centerNode);
      decorateEdge(graph_, edge, callerEdgeDecoration);
    }
  }

  _subgraphs.clear();
}

std::string CsharpDiagram::getFunctionCallLegend()
{
  util::LegendBuilder builder("Function call diagram");

  builder.addNode("center function", centerNodeDecoration);
  builder.addNode("called function", calleeNodeDecoration);
  builder.addNode("caller function", callerNodeDecoration);
  builder.addEdge("called", calleeEdgeDecoration);
  builder.addEdge("caller", callerEdgeDecoration);

  return builder.getOutput();
}

util::Graph::Node CsharpDiagram::addNode(
  util::Graph& graph_,
  const AstNodeInfo& nodeInfo_)
{
  util::Graph::Node node
    = graph_.getOrCreateNode(nodeInfo_.id,
      addSubgraph(graph_, nodeInfo_.range.file));

  graph_.setNodeAttribute(node, "label", nodeInfo_.astNodeValue);

  return node;
}

util::Graph::Subgraph CsharpDiagram::addSubgraph(
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

  _subgraphs.insert(it, std::make_pair(fileId_, subgraph));

  return subgraph;
}

void CsharpDiagram::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void CsharpDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

void CsharpDiagram::decorateSubgraph(
  util::Graph& graph_,
  const util::Graph::Subgraph& subgraph_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setSubgraphAttribute(subgraph_, attr.first, attr.second);
}

const CsharpDiagram::Decoration CsharpDiagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const CsharpDiagram::Decoration CsharpDiagram::calleeNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "lightblue"}
};

const CsharpDiagram::Decoration CsharpDiagram::callerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "coral"}
};

const CsharpDiagram::Decoration CsharpDiagram::calleeEdgeDecoration = {
  {"color", "blue"}
};

const CsharpDiagram::Decoration CsharpDiagram::callerEdgeDecoration = {
  {"color", "red"}
};

}
}
}
