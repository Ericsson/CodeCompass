#include <util/legendbuilder.h>
#include <util/util.h>
#include "diagram.h"

namespace
{

/**
 * This function checks if the given container contains a specified value.
 * @param v_ A container to inspect.
 * @param val_ Value which will be searched.
 * @return True if the container contains the value.
 */
template <typename Cont>
bool contains(const Cont& c_, const typename Cont::value_type& val_)
{
  return std::find(c_.begin(), c_.end(), val_) != c_.end();
}

/**
 * This function wraps the content to a HTML tag and adds attributes to it.
 */
std::string graphHtmlTag(
  const std::string& tag_,
  const std::string& content_,
  const std::string& attr_ = "")
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

}

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

void Diagram::getDetailedClassDiagram(
  util::Graph& graph_,
  const core::AstNodeId& astNodeId_)
{
  std::vector<AstNodeInfo> nodes;

  graph_.setAttribute("rankdir", "BT");

  //--- Center node ---//

  _cppHandler.getReferences(
    nodes, astNodeId_, CppServiceHandler::DEFINITION, {});

  if (nodes.empty())
    return;

  AstNodeInfo nodeInfo = nodes.front();

  util::Graph::Node currentNode = addNode(graph_, nodeInfo);
  graph_.setAttribute(currentNode, "label",
    getDetailedClassNodeLabel(nodeInfo), true);
  graph_.setAttribute(currentNode, "shape", "none");

  nodes.clear();

  //--- Types from which the queried type inherits ---//

  _cppHandler.getReferences(nodes, nodeInfo.id,
    CppServiceHandler::INHERIT_FROM, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addNode(graph_, node);
    graph_.setAttribute(inheritNode, "label",
      getDetailedClassNodeLabel(node), true);
    graph_.setAttribute(inheritNode, "shape", "none");

    util::Graph::Edge edge = graph_.addEdge(currentNode, inheritNode);
    graph_.setAttribute(edge, "arrowhead", "empty");
  }

  nodes.clear();

  //--- Types by which the queried type is inherited ---//

  _cppHandler.getReferences(nodes, nodeInfo.id,
    CppServiceHandler::INHERIT_BY, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addNode(graph_, node);
    graph_.setAttribute(inheritNode, "label",
      getDetailedClassNodeLabel(node), true);
    graph_.setAttribute(inheritNode, "shape", "none");

    util::Graph::Edge edge = graph_.addEdge(inheritNode, currentNode);
    graph_.setAttribute(edge, "arrowhead", "empty");
  }
}

std::string Diagram::getDetailedClassNodeLabel(const AstNodeInfo& nodeInfo_)
{
  std::string colAttr = "border='0' align='left'";
  std::string label = "<table border='1' cellspacing='0'>";

  label.append(graphHtmlTag("tr", graphHtmlTag("td",
    graphHtmlTag("font",
    util::escapeHtml(nodeInfo_.astNodeValue), "color='white'"),
    "colspan='2' SIDES='B' bgcolor='#316ECF' align='center'")));

  std::vector<AstNodeInfo> nodes;

  //--- Data members of the class ---//

  _cppHandler.getReferences(nodes, nodeInfo_.id,
    CppServiceHandler::DATA_MEMBER, {});

  for (auto it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string visibility = visibilityToHtml(*it);
    std::string content = memberContentToHtml(*it,
      util::escapeHtml(it->astNodeValue + " : " + getProperty(it->id, "Type")));

    std::string attr = colAttr;
    if (it == nodes.end() - 1)
      attr = "border='1' align='left' SIDES='B'";

    label += graphHtmlTag("tr",
      graphHtmlTag("td", visibility, attr) +
      graphHtmlTag("td", content, attr));
  }

  nodes.clear();

  //--- Methods of the class ---//

  _cppHandler.getReferences(nodes, nodeInfo_.id,
    CppServiceHandler::METHOD, {});

  for (const AstNodeInfo& node : nodes)
  {
    std::string visibility = visibilityToHtml(node);

    // TODO: Constructor and Destructor signatures can be empty.
    std::string signature = getProperty(node.id, "Signature");

    if (!signature.empty())
    {
      std::string content = memberContentToHtml(node,
        util::escapeHtml(signature));

      label += graphHtmlTag("tr",
        graphHtmlTag("td", visibility, colAttr) +
        graphHtmlTag("td", content, colAttr));
    }
  }

  label.append("</table>");

  return label;
}

std::string Diagram::visibilityToHtml(const AstNodeInfo& node_)
{
  if (contains(node_.tags, "public"))
    return graphHtmlTag("font", "+", "color='green'");
  if (contains(node_.tags, "private"))
    return graphHtmlTag("font", "-", "color='red'");
  if (contains(node_.tags, "protected"))
    return graphHtmlTag("font", "#", "color='blue'");

  return "";
}

std::string Diagram::memberContentToHtml(
  const AstNodeInfo& node_,
  const std::string& content_)
{
  std::string startTags;
  std::string endTags;

  if (contains(node_.tags, "static"))
  {
    startTags += "<b>";
    endTags.insert(0, "</b>");
  }

  if (contains(node_.tags, "virtual"))
  {
    startTags += "<i>";
    endTags.insert(0, "</i>");
  }

  return startTags + util::escapeHtml(content_) + endTags;
}

std::string Diagram::getProperty(
  const core::AstNodeId& astNodeId_,
  const std::string& property_)
{
  std::map<std::string, std::string> properties;
  _cppHandler.getProperties(properties, astNodeId_);
  return properties[property_];
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
