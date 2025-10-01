#include <util/graph.h>
#include "graphpimpl.h"

namespace cc
{
namespace util
{

Graph::Graph(const std::string name_, bool directed_, bool strict_)
  : _graphPimpl(new GraphPimpl(name_, directed_, strict_)),
    _directed(directed_), _strict(strict_), _isSubgraph(false)
{
  setAttribute("fontsize", "11");
}

Graph::Graph(Graph&& other) noexcept
  : _ids(std::move(other._ids)),
    _directed(other._directed),
    _strict(other._strict),
    _isSubgraph(other._isSubgraph)
{
  std::swap(this->_graphPimpl, other._graphPimpl);
}

Graph::~Graph()
{
  delete _graphPimpl;
}

// TODO: layout algorithm
std::string Graph::dotToSvg(const std::string& graph_)
{
  GVC_t*    gvc   = gvContext();
  Agraph_t* graph = agmemread(const_cast<char*>(graph_.c_str()));

  gvLayout(gvc, graph, "dot");

  char** result        = new char*;
  unsigned int* length = new unsigned int;

  gvRenderData(gvc, graph, "dot", result, length);
  gvFreeLayout(gvc, graph);

  std::string res = *result;

  delete *result;
  delete length;

  agclose(graph);
  gvFreeContext(gvc);

  return res;
}

bool Graph::isDirected() const
{
  return _directed;
}

bool Graph::isStrict() const
{
  return _strict;
}

bool Graph::isSubGraph() const
{
  return _isSubgraph;
}

int Graph::nodeCount() const
{
  return agnnodes(_graphPimpl->_graph);
}

int Graph::edgeCount() const
{
  return agnedges(_graphPimpl->_graph);
}

Graph::Node Graph::createNode(const Subgraph& subgraph_)
{
  return getOrCreateNode(generateId(), subgraph_);
}

Graph::Node Graph::getOrCreateNode(
  const Graph::Node& id_,
  const Subgraph& subgraph_)
{
  std::string id = id_.empty() ? generateId() : id_;
  _ids.insert(id);

  agnode(
    subgraph_.empty()
      ? _graphPimpl->_graph
      : _graphPimpl->_subgMap[subgraph_],
    const_cast<char*>(id.c_str()),
    1);

  setNodeAttribute(id_, "fontsize", "11");
  setNodeAttribute(id_, "id", id);

  return id;
}

Graph::Edge Graph::createEdge(const Node& from_, const Node& to_)
{
  std::string id = generateId();

  _graphPimpl->_edgeMap[id] = agedge(
    _graphPimpl->_graph,
    agnode(_graphPimpl->_graph, const_cast<char*>(from_.c_str()), 0),
    agnode(_graphPimpl->_graph, const_cast<char*>(to_.c_str()), 0),
    const_cast<char*>(id.c_str()),
    1);

  setEdgeAttribute(id, "fontsize", "11");
  setEdgeAttribute(id, "id", id);

  return id;
}

Graph::Subgraph Graph::getOrCreateSubgraph(const std::string& id_)
{
  std::string id = id_.empty() ? generateId() : id_;
  _ids.insert(id);

  _graphPimpl->_subgMap[id]
    = agsubg(_graphPimpl->_graph, const_cast<char*>(id.c_str()), 1);

  return id;
}

bool Graph::hasEdge(const Node& from_, const Node& to_) const
{
  return agedge(
    _graphPimpl->_graph,
    agnode(_graphPimpl->_graph, const_cast<char*>(from_.c_str()), 0),
    agnode(_graphPimpl->_graph, const_cast<char*>(to_.c_str()), 0),
    0,
    0);
}

bool Graph::hasNode(const Node& node_) const
{
  return agnode(_graphPimpl->_graph, const_cast<char*>(node_.c_str()), 0);
}

void Graph::delNode(const Node& node_)
{
  agdelnode(
    _graphPimpl->_graph,
    agnode(_graphPimpl->_graph, const_cast<char*>(node_.c_str()), 0));

  _ids.erase(node_);
}

void Graph::delEdge(const Node& from_, const Node& to_)
{
  agdeledge(
    _graphPimpl->_graph,
    agedge(_graphPimpl->_graph,
      agnode(_graphPimpl->_graph, const_cast<char*>(from_.c_str()), 0),
      agnode(_graphPimpl->_graph, const_cast<char*>(to_.c_str()), 0),
      0,
      0));
}

void Graph::setAttribute(const std::string& key_, const std::string& value_)
{
  agsafeset(
    _graphPimpl->_graph,
    const_cast<char*>(key_.c_str()),
    const_cast<char*>(value_.c_str()),
    const_cast<char*>(""));
}

void Graph::setNodeAttribute(
  const Node& node_,
  const std::string& key_,
  const std::string& value_,
  bool html_)
{
  const char* value = value_.c_str();

  if (html_)
    value = agstrdup_html(_graphPimpl->_graph,
      const_cast<char*>(value_.c_str()));

  agsafeset(
    agnode(_graphPimpl->_graph, const_cast<char*>(node_.c_str()), 0),
    const_cast<char*>(key_.c_str()),
    const_cast<char*>(value),
    const_cast<char*>(""));
}

void Graph::setEdgeAttribute(
  const Edge& edge_,
  const std::string& key_,
  const std::string& value_,
  bool html_)
{
  const char* value = value_.c_str();

  if (html_)
    value = agstrdup_html(_graphPimpl->_graph,
      const_cast<char*>(value_.c_str()));

  agsafeset(
    _graphPimpl->_edgeMap[edge_],
    const_cast<char*>(key_.c_str()),
    const_cast<char*>(value),
    const_cast<char*>(""));
}

void Graph::setSubgraphAttribute(
  const Subgraph& graph_,
  const std::string& key_,
  const std::string& value_,
  bool html_)
{
  const char* value = value_.c_str();

  if (html_)
    value = agstrdup_html(_graphPimpl->_graph,
      const_cast<char*>(value_.c_str()));

  agsafeset(
    _graphPimpl->_subgMap[graph_],
    const_cast<char*>(key_.c_str()),
    const_cast<char*>(value),
    const_cast<char*>(""));
}

void Graph::setNodeAttribute(
  const Node& targetNode_,
  const Node& sourceNode_)
{
  agcopyattr(
    agnode(_graphPimpl->_graph, const_cast<char*>(sourceNode_.c_str()), 0),
    agnode(_graphPimpl->_graph, const_cast<char*>(targetNode_.c_str()), 0));
}

void Graph::setEdgeAttribute(
  const Edge& targetEdge_,
  const Edge& sourceEdge_)
{
  agcopyattr(
    _graphPimpl->_edgeMap[sourceEdge_],
    _graphPimpl->_edgeMap[targetEdge_]);
}

std::string Graph::getNodeAttribute(
  const Node& node_,
  const std::string& key_)
{
  auto ret =
    agget(agnode(_graphPimpl->_graph, const_cast<char*>(node_.c_str()), 0),
      const_cast<char*>(key_.c_str()));

  return ret ? ret : "";
}

std::string Graph::getEdgeAttribute(const Edge& edge_, const std::string& key_)
{
  auto ret =
    agget(_graphPimpl->_edgeMap[edge_], const_cast<char*>(key_.c_str()));

  return ret ? ret : "";
}

// TODO: layout algorithm
// TODO: It's almost the same as dotToSvg() -> should be extracted.
// TODO: Called twice after each other it segfaults.
std::string Graph::output(Graph::Format format_) const
{
  char** result        = new char*;
  unsigned int* length = new unsigned int;

  gvLayout(_graphPimpl->_gvc, _graphPimpl->_graph, "dot");

  const char* render_format;
  switch (format_) {
    case Graph::DOT:
      render_format = "dot";
      break;
    case Graph::SVG:
      render_format = "svg";
      break;
    case Graph::CAIRO_SVG:
      render_format = "svg:cairo";
      break;
    default:
      __builtin_unreachable();
      break;
  }

  gvRenderData(
    _graphPimpl->_gvc,
    _graphPimpl->_graph,
    render_format,
    result,
    length);

  gvFreeLayout(_graphPimpl->_gvc, _graphPimpl->_graph);

  std::string res = *result;

  delete *result;
  delete length;

  return res;
}

std::vector<Graph::Node> Graph::getChildren(const Node& node) const
{
  std::vector<Graph::Node> result;

  Agnode_t* n
    = agnode(_graphPimpl->_graph, const_cast<char*>(node.c_str()), 0);

  if (!n)
    return result;

  for (Agedge_t* edge = agfstout(_graphPimpl->_graph, n);
       edge;
       edge = agnxtout(_graphPimpl->_graph, edge))
    result.push_back(std::string(agnameof(aghead(edge))));

  return result;
}

std::vector<Graph::Node> Graph::getParents(const Node& node) const
{
  std::vector<Graph::Node> result;

  Agnode_t* n
    = agnode(_graphPimpl->_graph, const_cast<char*>(node.c_str()), 0);

  if (!n)
    return result;

  for (Agedge_t* edge = agfstin(_graphPimpl->_graph, n);
       edge;
       edge = agnxtin(_graphPimpl->_graph, edge))
    result.push_back(std::string(agnameof(agtail(edge))));

  return result;
}

std::string Graph::generateId()
{
  do
  {
    std::size_t i;
    for (i = 0; i < _currentId.size() && _currentId[i] == 'z'; ++i)
      _currentId[i] = 'a';

    if (i == _currentId.size())
      _currentId.push_back('a');
    else
      ++_currentId[i];
  } while (_ids.find(_currentId) != _ids.end());

  return _currentId;
}

} // util
} // cc

