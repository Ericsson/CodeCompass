#include <util/legendbuilder.h>

namespace cc
{
namespace util
{

LegendBuilder::LegendBuilder(const std::string& title_)
{
  _graph.setAttribute("rankdir", "LR");
  _graph.setAttribute("nodesep", "0.25");
  _root = _graph.addNode();
  _graph.setAttribute(_root, "label", "");
  _graph.setAttribute(_root, "shape", "none");
  _graph.setAttribute(_root, "width", "0.1");
  _graph.setAttribute("label", title_);
}

Graph::Node LegendBuilder::registerSubgraph(const Graph::Subgraph& sub_)
{
  Graph::Node first = _graph.addNode(sub_);
  Graph::Edge invis = _graph.addEdge(_root, first);
  _graph.setAttribute(invis, "style", "invis");

  _graph.setAttribute(first, "label", "");
  _graph.setAttribute(first, "shape", "none");
  _graph.setAttribute(first, "width", "0.01");

  return first;
}

std::string LegendBuilder::getOutput() const
{
  return _graph.output(Graph::SVG);
}

void LegendBuilder::addNode(
  const std::string& label_,
  const std::vector<std::pair<std::string, std::string>>& attrs_)
{
  Graph::Subgraph sub = _graph.addSubgraph(label_);

  Graph::Node ref1 = registerSubgraph(sub); // padding node
  _graph.setAttribute(sub, "rankdir", "LR");

  Graph::Node node = _graph.addNode(sub);
  _graph.setAttribute(node, "label", "");
  Graph::Edge edge = _graph.addEdge(ref1, node); // padding edge
  _graph.setAttribute(edge, "style", "invis");

  Graph::Node desc = _graph.addNode(sub);
  _graph.setAttribute(desc, "shape", "none");
  _graph.setAttribute(desc, "label", label_);

  Graph::Edge invis = _graph.addEdge(node, desc);
  _graph.setAttribute(invis, "style", "invis");

  for (const auto& attr : attrs_)
    _graph.setAttribute(node, attr.first, attr.second);
}


void LegendBuilder::addEdge(
  const std::string& label_,
  const std::vector<std::pair<std::string, std::string>>& attrs_)
{
  Graph::Subgraph sub = _graph.addSubgraph(label_);
  _graph.setAttribute(sub, "rankdir", "LR");

  Graph::Node ref1 = registerSubgraph(sub);

  Graph::Node ref2 = _graph.addNode(sub);
  _graph.setAttribute(ref2, "label", "");
  _graph.setAttribute(ref2, "shape", "none");
  _graph.setAttribute(ref2, "width", "0.01");
  Graph::Edge edge = _graph.addEdge(ref1, ref2);

  Graph::Node desc = _graph.addNode(sub);
  _graph.setAttribute(desc, "shape", "none");
  _graph.setAttribute(desc, "label", label_);

  Graph::Edge invis = _graph.addEdge(ref2, desc);
  _graph.setAttribute(invis, "style", "invis");

  for (const auto& attr : attrs_)
    _graph.setAttribute(edge, attr.first, attr.second);
}

Graph::Subgraph LegendBuilder::addSubgraph(
  const std::string& label_,
  Graph::Node& hook_)
{
  Graph::Subgraph sub = _graph.addSubgraph(label_);
  _graph.setAttribute(sub, "rankdir", "LR");
  Graph::Node ref = registerSubgraph(sub); // padding node

  hook_ = ref;
  return sub;
}

void LegendBuilder::setStyle(
  const Graph::Node& node_,
  const std::vector<std::pair<std::string, std::string>>& attrs_)
{
  for (const auto& attr : attrs_)
    _graph.setAttribute(node_, attr.first, attr.second);
}

void LegendBuilder::setStyle(
  const Graph::Edge& edge_,
  const std::vector<std::pair<std::string, std::string>>& attrs_)
{
  for (const auto& attr : attrs_)
    _graph.setAttribute(edge_, attr.first, attr.second);
}

} // util
} // cc
