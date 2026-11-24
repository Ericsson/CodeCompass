#include <algorithm>
#include <cctype>
#include <regex>

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

const std::vector<std::string> accessibilityModifiers = {
  "public", "private", "protected", "internal"};

/**
 * All member modifiers. In the class diagram the visibility and the static and
 * virtual members are shown by formatting, so the keywords are not needed.
 */
const std::vector<std::string> memberModifiers = {
  "public", "private", "protected", "internal", "static", "virtual",
  "abstract", "override", "sealed", "new", "extern", "unsafe", "partial",
  "async", "readonly", "required", "volatile", "const"};

/**
 * Removes the leading attributes and the given leading modifiers from the
 * declaration of a node, and cuts the signature before the body.
 */
std::string toSignature(
  std::string str_,
  const std::vector<std::string>& modifiers_ = accessibilityModifiers)
{
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

  // Modifiers are only removed as whole leading words, so names like
  // "publicKey" are kept.
  bool removed = true;
  while (removed)
  {
    removed = false;

    for (const auto& tag : modifiers_)
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

/**
 * Returns the signature of a member for the class diagram: without
 * attributes, modifiers, body and constructor initializer (": base(...)").
 */
std::string toMemberSignature(const std::string& str_, const std::string& kind_)
{
  // Multi-line parameter lists are joined into one line.
  static const std::regex whitespace("\\s+");
  static const std::regex afterOpenParen("\\( ");
  static const std::regex beforeCloseParen(" \\)");
  std::string signature = std::regex_replace(
    toSignature(str_, memberModifiers), whitespace, " ");
  signature = std::regex_replace(signature, afterOpenParen, "(");
  signature = std::regex_replace(signature, beforeCloseParen, ")");

  // Interface, abstract and delegate members end with a semicolon.
  if (!signature.empty() && signature.back() == ';')
  {
    signature.pop_back();
    boost::trim_right(signature);
  }

  if (kind_ == "ConstructorDeclaration")
  {
    std::size_t open = signature.find('(');
    int depth = 0;

    for (std::size_t i = open; open != std::string::npos && i < signature.size(); ++i)
      if (signature[i] == '(')
        ++depth;
      else if (signature[i] == ')' && --depth == 0)
      {
        signature.erase(i + 1);
        break;
      }
  }

  return signature;
}

/**
 * Removes the namespace qualifiers from a type name, e.g.
 * "System.Collections.Generic.List<MyApp.Models.Item>" -> "List<Item>".
 */
std::string toShortTypeName(const std::string& type_)
{
  static const std::regex qualifier("[A-Za-z_][A-Za-z0-9_]*\\.");
  return std::regex_replace(type_, qualifier, "");
}

/**
 * This function wraps the content to a HTML tag and adds attributes to it.
 */
std::string graphHtmlTag(
  const std::string& tag_,
  const std::string& content_,
  const std::string& attr_ = "")
{
  return "<" + tag_ + " " + attr_ + ">" + content_ + "</" + tag_ + ">";
}

bool contains(const std::vector<std::string>& tags_, const std::string& tag_)
{
  return std::find(tags_.begin(), tags_.end(), tag_) != tags_.end();
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

void CsharpDiagram::getDetailedClassDiagram(
  util::Graph& graph_,
  CsharpServiceHandler& csharpHandler_,
  const core::AstNodeId& astNodeId_)
{
  std::vector<AstNodeInfo> nodes;

  graph_.setAttribute("rankdir", "BT");

  //--- Center node ---//

  AstNodeInfo nodeInfo;
  csharpHandler_.getAstNodeInfo(nodeInfo, astNodeId_);

  util::Graph::Node currentNode = addClassNode(graph_, csharpHandler_, nodeInfo);

  //--- Types from which the queried type inherits ---//

  csharpHandler_.getReferences(nodes, nodeInfo.id,
    CsharpServiceHandler::INHERIT_FROM, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addClassNode(graph_, csharpHandler_, node);

    if (!graph_.hasEdge(currentNode, inheritNode))
    {
      util::Graph::Edge edge = graph_.createEdge(currentNode, inheritNode);
      decorateEdge(graph_, edge, inheritClassEdgeDecoration);
    }
  }

  nodes.clear();

  //--- Types by which the queried type is inherited ---//

  csharpHandler_.getReferences(nodes, nodeInfo.id,
    CsharpServiceHandler::INHERIT_BY, {});

  for (const AstNodeInfo& node : nodes)
  {
    util::Graph::Node inheritNode = addClassNode(graph_, csharpHandler_, node);

    if (!graph_.hasEdge(inheritNode, currentNode))
    {
      util::Graph::Edge edge = graph_.createEdge(inheritNode, currentNode);
      decorateEdge(graph_, edge, inheritClassEdgeDecoration);
    }
  }

  _subgraphs.clear();
}

std::string CsharpDiagram::getDetailedClassLegend()
{
  util::LegendBuilder builder("Detailed Class Diagram");

  builder.addNode("class", classNodeDecoration);
  builder.addNode("public member", {{"shape", "none"},
    {"label", graphHtmlTag("font", "+", "color='green'")}}, true);
  builder.addNode("private member", {{"shape", "none"},
    {"label", graphHtmlTag("font", "-", "color='red'")}}, true);
  builder.addNode("protected member", {{"shape", "none"},
    {"label", graphHtmlTag("font", "#", "color='blue'")}}, true);
  builder.addNode("internal member", {{"shape", "none"},
    {"label", graphHtmlTag("font", "~", "color='darkorange'")}}, true);
  builder.addNode("static method or data member", {{"shape", "none"},
    {"label", "<b>static</b>"}}, true);
  builder.addNode("virtual, abstract or override method", {{"shape", "none"},
    {"label", "<i>virtual</i>"}}, true);
  builder.addEdge("inheritance", inheritClassEdgeDecoration);

  return builder.getOutput();
}

util::Graph::Node CsharpDiagram::addClassNode(
  util::Graph& graph_,
  CsharpServiceHandler& csharpHandler_,
  const AstNodeInfo& nodeInfo_)
{
  util::Graph::Node node = addNode(graph_, nodeInfo_);

  graph_.setNodeAttribute(node, "label",
    getDetailedClassNodeLabel(csharpHandler_, nodeInfo_), true);
  graph_.setNodeAttribute(node, "shape", "none");

  return node;
}

std::string CsharpDiagram::getDetailedClassNodeLabel(
  CsharpServiceHandler& csharpHandler_,
  const AstNodeInfo& nodeInfo_)
{
  std::string colAttr = "border='0' align='left'";
  std::string label = "<table border='1' cellspacing='0'>";

  // The AST node value of a type is its whole source text, so the name is
  // taken from its properties.
  std::map<std::string, std::string> properties;
  csharpHandler_.getProperties(properties, nodeInfo_.id);
  std::string name = boost::trim_copy(properties["Name"]);

  label.append(graphHtmlTag("tr", graphHtmlTag("td",
    graphHtmlTag("font", util::escapeHtml(name), "color='white'"),
    "colspan='2' SIDES='B' bgcolor='#316ECF' align='center'")));

  //--- Data members of the type: fields, then properties ---//

  std::vector<AstNodeInfo> nodes;
  csharpHandler_.getReferences(nodes, nodeInfo_.id,
    CsharpServiceHandler::LOCAL_VAR, {});

  std::vector<AstNodeInfo> propertyNodes;
  csharpHandler_.getReferences(propertyNodes, nodeInfo_.id,
    CsharpServiceHandler::DATA_MEMBER, {});
  nodes.insert(nodes.end(), propertyNodes.begin(), propertyNodes.end());

  for (auto it = nodes.begin(); it != nodes.end(); ++it)
  {
    std::map<std::string, std::string> memberProperties;
    csharpHandler_.getProperties(memberProperties, it->id);

    std::string content = boost::trim_copy(memberProperties["Name"]);
    std::string type = toShortTypeName(
      boost::trim_copy(memberProperties["Qualified Type"]));
    if (!type.empty())
      content += " : " + type;

    std::string attr = colAttr;
    if (it == nodes.end() - 1)
      attr = "border='1' align='left' SIDES='B'";

    label += graphHtmlTag("tr",
      graphHtmlTag("td", visibilityToHtml(*it), attr) +
      graphHtmlTag("td",
        memberContentToHtml(*it, util::escapeHtml(content)), attr));
  }

  nodes.clear();

  //--- Methods of the type ---//

  csharpHandler_.getReferences(nodes, nodeInfo_.id,
    CsharpServiceHandler::METHOD, {});

  for (const AstNodeInfo& node : nodes)
  {
    // Property accessors are shown by their properties.
    if (contains(node.tags, "accessor"))
      continue;

    std::string signature =
      toMemberSignature(node.astNodeValue, node.astNodeType);

    if (!signature.empty())
      label += graphHtmlTag("tr",
        graphHtmlTag("td", visibilityToHtml(node), colAttr) +
        graphHtmlTag("td",
          memberContentToHtml(node, util::escapeHtml(signature)), colAttr));
  }

  label.append("</table>");

  return label;
}

std::string CsharpDiagram::visibilityToHtml(const AstNodeInfo& node_)
{
  if (contains(node_.tags, "public"))
    return graphHtmlTag("font", "+", "color='green'");
  if (contains(node_.tags, "private"))
    return graphHtmlTag("font", "-", "color='red'");
  if (contains(node_.tags, "protected"))
    return graphHtmlTag("font", "#", "color='blue'");
  if (contains(node_.tags, "internal"))
    return graphHtmlTag("font", "~", "color='darkorange'");

  return "";
}

std::string CsharpDiagram::memberContentToHtml(
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

  return startTags + content_ + endTags;
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

const CsharpDiagram::Decoration CsharpDiagram::classNodeDecoration = {
  {"shape", "box"}
};

const CsharpDiagram::Decoration CsharpDiagram::inheritClassEdgeDecoration = {
  {"arrowhead", "empty"}
};

}
}
}
