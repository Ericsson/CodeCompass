#include <iostream>

#include <util/legendbuilder.h>
#include "diagram.h"

namespace cc
{
namespace service
{
namespace language
{

Diagram::Diagram(
  std::shared_ptr<odb::database> db_,
  const boost::program_options::variables_map& config_)
    : _cppHandler(db_, config_), _projectHandler(db_, config_)
{
}

void Diagram::getFunctionCallDiagram(
  util::Graph& graph_,
  const core::AstNodeId& astNodeId_)
{
  std::map<core::AstNodeId, util::Graph::Node> visitedNodes;
  std::vector<AstNodeInfo> nodes;

  graph_.setAttribute("rankdir", "LR");

  //--- Center node ---//

  _cppHandler.getReferences(
    nodes, astNodeId_, CppServiceHandler::DEFINITION, {});

  if (nodes.empty())
    return;

  util::Graph::Node centerNode = addNode(graph_, nodes.front());
  decorate(graph_, centerNode, centerNodeDecoration);
  visitedNodes[astNodeId_] = centerNode;

  //--- Callees ---//

  nodes.clear();
  _cppHandler.getReferences(nodes, astNodeId_, CppServiceHandler::CALLEE, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node calleeNode;

    auto it = visitedNodes.find(node.id);
    if (it == visitedNodes.end())
    {
      calleeNode = addNode(graph_, node);
      decorate(graph_, calleeNode, calleeNodeDecoration);
      visitedNodes.insert(it, std::make_pair(node.id, calleeNode));
    }
    else
      calleeNode = it->second;

    if (!graph_.hasEdge(centerNode, calleeNode))
    {
      util::Graph::Edge edge = graph_.addEdge(centerNode, calleeNode);
      decorate(graph_, edge, calleeEdgeDecoration);
    }
  }

  //--- Callers ---//

  nodes.clear();
  _cppHandler.getReferences(nodes, astNodeId_, CppServiceHandler::CALLER, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node callerNode;

    auto it = visitedNodes.find(node.id);
    if (it == visitedNodes.end())
    {
      callerNode = addNode(graph_, node);
      decorate(graph_, callerNode, callerNodeDecoration);
      visitedNodes.insert(it, std::make_pair(node.id, callerNode));
    }
    else
      callerNode = it->second;

    if (!graph_.hasEdge(callerNode, centerNode))
    {
      util::Graph::Edge edge = graph_.addEdge(callerNode, centerNode);
      decorate(graph_, edge, callerEdgeDecoration);
    }
  }

  _subgraphs.clear();
}

util::Graph::Node Diagram::addNode(
  util::Graph& graph_,
  const AstNodeInfo& nodeInfo_)
{
  util::Graph::Node node
    = graph_.addNode(addSubgraph(graph_, nodeInfo_.range.file));

  graph_.setAttribute(node, "id", nodeInfo_.id);
  graph_.setAttribute(node, "label", nodeInfo_.astNodeValue);

  return node;
}

std::string Diagram::getFunctionCallLegend()
{
  util::LegendBuilder builder("Function Call Diagram");

  builder.addNode("center function", centerNodeDecoration);
  builder.addNode("called function", calleeNodeDecoration);
  builder.addNode("caller function", callerNodeDecoration);
  builder.addNode("virtual function", virtualNodeDecoration);
  builder.addEdge("called", calleeEdgeDecoration);
  builder.addEdge("caller", callerEdgeDecoration);

  return builder.getOutput();
}

util::Graph::Subgraph Diagram::addSubgraph(
  util::Graph& graph_,
  const core::FileId& fileId_)
{
  auto it = _subgraphs.find(fileId_);

  if (it != _subgraphs.end())
    return it->second;

  core::FileInfo fileInfo;
  _projectHandler.getFileInfo(fileInfo, fileId_);

  util::Graph::Subgraph subgraph
    = graph_.addSubgraph("cluster_" + fileInfo.path);

  graph_.setAttribute(subgraph, "id", fileInfo.id);
  graph_.setAttribute(subgraph, "label", fileInfo.path);

  _subgraphs.insert(it, std::make_pair(fileInfo.path, subgraph));

  return subgraph;
}

const Diagram::Decoration Diagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const Diagram::Decoration Diagram::calleeNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "lightblue"}
};

const Diagram::Decoration Diagram::callerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "coral"}
};

const Diagram::Decoration Diagram::virtualNodeDecoration = {
  {"shape", "diamond"},
  {"style", "filled"},
  {"fillcolor", "cyan"}
};

const Diagram::Decoration Diagram::calleeEdgeDecoration = {
  {"color", "blue"}
};

const Diagram::Decoration Diagram::callerEdgeDecoration = {
  {"color", "red"}
};

}
}
}
