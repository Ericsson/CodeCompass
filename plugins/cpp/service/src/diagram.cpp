#include <iostream>

#include <util/legendbuilder.h>
#include "diagram.h"

namespace
{

/**
 * This function escapes a string using html escapes characters.
 * @param str_ String which will be escaped.
 * @return Escaped html sequence.
 */
std::string escapeHtml(const std::string& str_)
{
  std::string ret;
  for(const char& c : str_)
    switch(c)
    {
    case '<':
      ret += "&lt;";
      break;
    case '>':
      ret += "&gt;";
      break;
    case '&':
      ret += "&amp;";
      break;
    default:
      ret += c;
    }
  return ret;
}

/**
 * This function check that a vector contain a specified value.
 * @param v_ Vector of strings.
 * @param val_ Value which will be searched.
 * @return True if the vector contain the value.
 */
bool contain(
  const std::vector<std::string>& v_,
  const std::string& val_)
{
  return std::find(v_.begin(), v_.end(), val_) != v_.end();
}

/**
 * This function wraps the content to a html font tag and add attributes to it.
 * @param content_ String which will be wrapped to font tag.
 * @param attr_ Attributes of font tag. For more information see:
 * http://www.graphviz.org/doc/info/shapes.html#html
 * @return Content wrapped by font tag.
 */
std::string graphHtmlFontTag(
  const std::string& content_,
  const std::string& attr_ = "")
{
  return std::string("<font ")
    .append(attr_)
    .append(">")
    .append(content_)
    .append("</font>");
}

/**
 * This function wraps the content to a html table row tag and add attributes
 * to it.
 * @param content_ String which will be wrapped to html table tr tag.
 */
std::string graphHtmlTableRowTag(const std::string& content_)
{
  return std::string("<tr>")
    .append(content_)
    .append("</tr>");
}

/**
 * This function wraps the content to a html table column tag and add attributes
 * to it. In attribute we can specify an align, color, bgcolor or other options.
 * For more information about attributes see:
 * http://www.graphviz.org/doc/info/shapes.html#html
 */
std::string graphHtmlTableColTag(
  const std::string& content_,
  std::string attr_)
{
  return std::string("<td ")
    .append(attr_)
    .append(">")
    .append(content_)
    .append("</td>");
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
  graph_.setHtmlLabel(currentNode, getDetailedClassNodeLabel(nodeInfo));
  graph_.setAttribute(currentNode, "shape", "none");

  nodes.clear();

  //--- Types from which the queried type inherits. ---//

  _cppHandler.getReferences(nodes, nodeInfo.id,
    CppServiceHandler::INHERIT_FROM, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addNode(graph_, node);
    graph_.setHtmlLabel(inheritNode, getDetailedClassNodeLabel(node));
    graph_.setAttribute(inheritNode, "shape", "none");

    util::Graph::Edge edge = graph_.addEdge(currentNode, inheritNode);
    graph_.setAttribute(edge, "arrowhead", "empty");
  }

  nodes.clear();

  //--- Types by which the queried type is inherited. ---//

  _cppHandler.getReferences(nodes, nodeInfo.id,
    CppServiceHandler::INHERIT_BY, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addNode(graph_, node);
    graph_.setHtmlLabel(inheritNode, getDetailedClassNodeLabel(node));
    graph_.setAttribute(inheritNode, "shape", "none");

    util::Graph::Edge edge = graph_.addEdge(inheritNode, currentNode);
    graph_.setAttribute(edge, "arrowhead", "empty");
  }
}

std::string Diagram::getDetailedClassNodeLabel(const AstNodeInfo& nodeInfo_)
{
  std::string colAttr = "border='0' align='left'";
  std::string label = "<table border='1' cellspacing='0'>";

  label.append(graphHtmlTableRowTag(graphHtmlTableColTag(
    graphHtmlFontTag(escapeHtml(nodeInfo_.astNodeValue), "color='white'"),
    "colspan='2' SIDES='B' bgcolor='#316ECF' align='center'")));

  std::vector<AstNodeInfo> nodes;

  //--- Data members of the class ---//

  _cppHandler.getReferences(nodes, nodeInfo_.id,
    CppServiceHandler::DATA_MEMBER, {});

  for (auto it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::string visibility = visibilityToHtml(*it);
    std::string content = memberContentToHtml(*it,
      escapeHtml(it->astNodeValue + " : " + getProperty(it->id, "Type")));

    std::string attr = colAttr;
    if(it == nodes.end() - 1)
      attr = "border='1' align='left' SIDES='B'";

    label += graphHtmlTableRowTag(
      graphHtmlTableColTag(visibility, attr) +
      graphHtmlTableColTag(content, attr));
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

    if(!signature.empty())
    {
      std::string content = memberContentToHtml(node,
        escapeHtml(signature));

      label += graphHtmlTableRowTag(
        graphHtmlTableColTag(visibility, colAttr) +
        graphHtmlTableColTag(content, colAttr));
    }
  }

  label.append("</table>");

  return label;
}


std::string Diagram::visibilityToHtml(const AstNodeInfo& node_)
{
  if(contain(node_.tags, "public"))
    return graphHtmlFontTag("+", "color='green'");
  if(contain(node_.tags, "private"))
    return graphHtmlFontTag("-", "color='red'");
  if(contain(node_.tags, "protected"))
    return graphHtmlFontTag("#", "color='blue'");

  return "";
}

std::string Diagram::memberContentToHtml(
  const AstNodeInfo& node_,
  const std::string& content_)
{
  std::string startTags;
  std::string endTags;

  if(contain(node_.tags, "static"))
  {
    startTags += "<b>";
    endTags.insert(0, "</b>");
  }

  if(contain(node_.tags, "virtual"))
  {
    startTags += "<i>";
    endTags.insert(0, "</i>");
  }

  return startTags + content_ + endTags;
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
