#include <util/legendbuilder.h>

namespace cc
{
namespace util
{

LegendBuilder::LegendBuilder(const std::string& title_)
{
  _graph.setAttribute("rankdir", "LR");
  _graph.setAttribute("nodesep", "0.25");
  _root = _graph.createNode();
  _graph.setNodeAttribute(_root, "label", "");
  _graph.setNodeAttribute(_root, "shape", "none");
  _graph.setNodeAttribute(_root, "width", "0.1");
  _graph.setAttribute("label", title_);
}

Graph::Node LegendBuilder::registerSubgraph(const Graph::Subgraph& sub_)
{
  Graph::Node first = _graph.createNode(sub_);
  Graph::Edge invis = _graph.createEdge(_root, first);
  _graph.setEdgeAttribute(invis, "style", "invis");

  _graph.setNodeAttribute(first, "label", "");
  _graph.setNodeAttribute(first, "shape", "none");
  _graph.setNodeAttribute(first, "width", "0.01");

  return first;
}

std::string LegendBuilder::getOutput() const
{
  return _graph.output(Graph::SVG);
}

void LegendBuilder::addNode(
  const std::string& label_,
  const std::vector<std::pair<std::string, std::string>>& attrs_,
  bool html_)
{
  Graph::Subgraph sub = _graph.getOrCreateSubgraph(label_);

  Graph::Node ref1 = registerSubgraph(sub); // padding node
  _graph.setSubgraphAttribute(sub, "rankdir", "LR");

  Graph::Node node = _graph.createNode(sub);
  _graph.setNodeAttribute(node, "label", "");
  Graph::Edge edge = _graph.createEdge(ref1, node); // padding edge
  _graph.setEdgeAttribute(edge, "style", "invis");

  Graph::Node desc = _graph.createNode(sub);
  _graph.setNodeAttribute(desc, "shape", "none");
  _graph.setNodeAttribute(desc, "label", label_);

  Graph::Edge invis = _graph.createEdge(node, desc);
  _graph.setEdgeAttribute(invis, "style", "invis");

  for (const auto& attr : attrs_)
    _graph.setNodeAttribute(node, attr.first, attr.second, html_);
}

void LegendBuilder::addEdge(
  const std::string& label_,
  const std::vector<std::pair<std::string, std::string>>& attrs_,
  bool html_)
{
  Graph::Subgraph sub = _graph.getOrCreateSubgraph(label_);
  _graph.setSubgraphAttribute(sub, "rankdir", "LR");

  Graph::Node ref1 = registerSubgraph(sub);

  Graph::Node ref2 = _graph.createNode(sub);
  _graph.setNodeAttribute(ref2, "label", "");
  _graph.setNodeAttribute(ref2, "shape", "none");
  _graph.setNodeAttribute(ref2, "width", "0.01");
  Graph::Edge edge = _graph.createEdge(ref1, ref2);

  Graph::Node desc = _graph.createNode(sub);
  _graph.setNodeAttribute(desc, "shape", "none");
  _graph.setNodeAttribute(desc, "label", label_);

  Graph::Edge invis = _graph.createEdge(ref2, desc);
  _graph.setEdgeAttribute(invis, "style", "invis");

  for (const auto& attr : attrs_)
    _graph.setEdgeAttribute(edge, attr.first, attr.second, html_);
}

Graph::Subgraph LegendBuilder::addSubgraph(
  const std::string& label_,
  Graph::Node& hook_)
{
  Graph::Subgraph sub = _graph.getOrCreateSubgraph(label_);
  _graph.setSubgraphAttribute(sub, "rankdir", "LR");
  Graph::Node ref = registerSubgraph(sub); // padding node

  hook_ = ref;
  return sub;
}

void LegendBuilder::setNodeStyle(
  const Graph::Node& node_,
  const std::vector<std::pair<std::string, std::string>>& attrs_,
  bool html_)
{
  for (const auto& attr : attrs_)
    _graph.setNodeAttribute(node_, attr.first, attr.second, html_);
}

void LegendBuilder::setEdgeStyle(
  const Graph::Edge& edge_,
  const std::vector<std::pair<std::string, std::string>>& attrs_,
  bool html_)
{
  for (const auto& attr : attrs_)
    _graph.setEdgeAttribute(edge_, attr.first, attr.second, html_);
}

} // util
} // cc
