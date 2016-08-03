#include <vector>

#include <util/diagram/legendbuilder.h>
#include <util/graph/graph.h>

namespace cc
{
namespace util
{
namespace diagram
{

util::Graph::Node LegendBuilder::registerSubgraph(util::Graph::Subgraph & sub)
{
  util::Graph::Node first = graph.addNode(sub);
  util::Graph::Edge invis = graph.addEdge(root, first);
  graph.setAttribute(invis, "style", "invis");

  graph.setAttribute(first, "label", "");
  graph.setAttribute(first, "shape", "none");
  graph.setAttribute(first, "width", "0.01");

  return first;
}

LegendBuilder::LegendBuilder()
{
  graph.setAttribute("rankdir", "LR");
  graph.setAttribute("nodesep", "0.25");
  root = graph.addNode();
  graph.setAttribute(root, "label", "");
  graph.setAttribute(root, "shape", "none");
  graph.setAttribute(root, "width", "0.1");
}

LegendBuilder::LegendBuilder(const std::string&  title)
{
  graph.setAttribute("rankdir", "LR");
  graph.setAttribute("nodesep", "0.25");
  root = graph.addNode();
  graph.setAttribute(root, "label", "");
  graph.setAttribute(root, "shape", "none");
  graph.setAttribute(root, "width", "0.1");
  graph.setAttribute("label", title);

}

std::string LegendBuilder::getOutput()
{
  return graph.output(util::Graph::SVG);
}

void LegendBuilder::setStyle(util::Graph::Node node, const std::vector<std::pair<std::string, std::string>> & attrs)
{

  for(const auto & attr : attrs)
  {
    graph.setAttribute(node, attr.first, attr.second);
  }
}

void LegendBuilder::setStyle(util::Graph::Edge edge, const std::vector<std::pair<std::string, std::string>> & attrs)
{

  for(const auto & attr : attrs)
  {
    graph.setAttribute(edge, attr.first, attr.second);
  }
}

void LegendBuilder::addNode(const std::string & label, const std::vector<std::pair<std::string, std::string>> & attrs)
{
  util::Graph::Subgraph sub = graph.addSubgraph(label);

  util::Graph::Node ref1 = registerSubgraph(sub); // padding node
  graph.setAttribute(sub,"rankdir","LR");

  util::Graph::Node node = graph.addNode(sub);
  graph.setAttribute(node, "label", "");
  util::Graph::Edge edge = graph.addEdge(ref1, node); // padding edge
  graph.setAttribute(edge, "style", "invis");

  util::Graph::Node desc = graph.addNode(sub);
  graph.setAttribute(desc, "shape", "none");
  graph.setAttribute(desc, "label", label);

  util::Graph::Edge invis = graph.addEdge(node, desc);
  graph.setAttribute(invis, "style", "invis");

  for(const auto & attr : attrs)
  {
    graph.setAttribute(node, attr.first, attr.second);
  }
}


void LegendBuilder::addEdge(const std::string & label, const std::vector<std::pair<std::string, std::string>> & attrs)
{
  util::Graph::Subgraph sub = graph.addSubgraph(label);
  graph.setAttribute(sub,"rankdir","LR");

  util::Graph::Node ref1 = registerSubgraph(sub);

  util::Graph::Node ref2 = graph.addNode(sub);
  graph.setAttribute(ref2, "label", "");
  graph.setAttribute(ref2, "shape", "none");
  graph.setAttribute(ref2, "width", "0.01");
  util::Graph::Edge edge = graph.addEdge(ref1, ref2);

  util::Graph::Node desc = graph.addNode(sub);
  graph.setAttribute(desc, "shape", "none");
  graph.setAttribute(desc, "label", label);

  util::Graph::Edge invis = graph.addEdge(ref2, desc);
  graph.setAttribute(invis, "style", "invis");

  for(const auto & attr : attrs)
  {
    graph.setAttribute(edge, attr.first, attr.second);
  }
}

util::Graph* LegendBuilder::getGraph(){
  return &graph;
}


util::Graph::Subgraph LegendBuilder::addSubgraph(const std::string & label,util::Graph::Node* hook)
{

  util::Graph::Subgraph sub = graph.addSubgraph(label);
  graph.setAttribute(sub,"rankdir","LR");
  util::Graph::Node ref1 = registerSubgraph(sub); // padding node

  *hook=ref1;
  return sub;
}

} // diagram
} // util
} // cc
