#include <algorithm>
#include <cctype>

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
 * Returns the position after the attribute section (e.g. [Route("[x]")])
 * starting at pos_, or std::string::npos if the brackets are unbalanced.
 */
std::size_t skipAttribute(const std::string& str_, std::size_t pos_)
{
  int depth = 0;
  bool inString = false;

  for (; pos_ < str_.size(); ++pos_)
  {
    char c = str_[pos_];

    if (inString)
    {
      if (c == '\\')
        ++pos_;
      else if (c == '"')
        inString = false;
    }
    else if (c == '"')
      inString = true;
    else if (c == '[')
      ++depth;
    else if (c == ']' && --depth == 0)
      return pos_ + 1;
  }

  return std::string::npos;
}

/**
 * Removes the leading attributes and accessibility modifiers from the
 * declaration of a node, and cuts the signature before the body.
 */
std::string toSignature(std::string str_)
{
  static const std::vector<std::string> accessibilityTags = {
    "public", "private", "protected", "internal"};

  boost::trim_left(str_);

  // Attributes may share a line with the declaration, so they are removed
  // by bracket matching instead of dropping whole lines.
  while (!str_.empty() && str_[0] == '[')
  {
    std::size_t end = skipAttribute(str_, 0);
    if (end == std::string::npos)
      break;

    str_.erase(0, end);
    boost::trim_left(str_);
  }

  // Accessibility modifiers are only removed as whole leading words, so names
  // like "publicKey" are kept.
  bool removed = true;
  while (removed)
  {
    removed = false;

    for (const auto& tag : accessibilityTags)
      if (boost::starts_with(str_, tag) &&
          (str_.size() == tag.size() ||
           std::isspace(static_cast<unsigned char>(str_[tag.size()]))))
      {
        str_.erase(0, tag.size());
        boost::trim_left(str_);
        removed = true;
      }
  }

  // Cut before a block body or an expression body.
  str_ = str_.substr(0, std::min(str_.find('{'), str_.find("=>")));
  boost::trim(str_);

  return str_;
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
  // A node is decorated only when it is first added, so e.g. a recursive
  // center node keeps its own decoration (same as in the C++ plugin).
  std::map<core::AstNodeId, util::Graph::Node> visitedNodes;

  graph_.setAttribute("rankdir", "LR");

  AstNodeInfo centerInfo = centerNodeInfo_;
  centerInfo.astNodeValue = toSignature(centerInfo.astNodeValue);
  util::Graph::Node centerNode = addNode(graph_, centerInfo);
  decorateNode(graph_, centerNode, centerNodeDecoration);
  visitedNodes[centerInfo.id] = centerNode;

  for (const auto& calleeNodeInfo : calleeNodeInfos_)
  {
    util::Graph::Node calleeNode;

    auto it = visitedNodes.find(calleeNodeInfo.id);
    if (it == visitedNodes.end())
    {
      AstNodeInfo calleeInfo = calleeNodeInfo;
      calleeInfo.astNodeValue = toSignature(calleeInfo.astNodeValue);
      calleeNode = addNode(graph_, calleeInfo);
      decorateNode(graph_, calleeNode, calleeNodeDecoration);
      visitedNodes.insert(it, std::make_pair(calleeInfo.id, calleeNode));
    }
    else
      calleeNode = it->second;

    if (!graph_.hasEdge(centerNode, calleeNode))
    {
      util::Graph::Edge edge = graph_.createEdge(centerNode, calleeNode);
      decorateEdge(graph_, edge, calleeEdgeDecoration);
    }
  }

  for (const auto& callerNodeInfo : callerNodeInfos_)
  {
    util::Graph::Node callerNode;

    auto it = visitedNodes.find(callerNodeInfo.id);
    if (it == visitedNodes.end())
    {
      AstNodeInfo callerInfo = callerNodeInfo;
      callerInfo.astNodeValue = toSignature(callerInfo.astNodeValue);
      callerNode = addNode(graph_, callerInfo);
      decorateNode(graph_, callerNode, callerNodeDecoration);
      visitedNodes.insert(it, std::make_pair(callerInfo.id, callerNode));
    }
    else
      callerNode = it->second;

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
