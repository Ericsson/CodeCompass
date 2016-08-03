#include <util/graph/graphpimpl.h>
#include <util/graph/graph.h>

namespace cc
{
namespace util
{

Graph::Graph(std::string name_, bool directed_, bool strict_)
  : _graphPimpl(new GraphPimpl(name_, directed_, strict_)),
    _directed(directed_), _strict(strict_), _isSubgraph(false)
{
  setAttribute("fontsize", "11");
}

Graph::~Graph()
{
  delete _graphPimpl;
}

// TODO: layout algoritmus
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

Graph::Node Graph::addNode(Subgraph subgraph_)
{
  std::string id = generateId();
  
  agnode(subgraph_.id.empty() ? _graphPimpl->_graph
                              : _graphPimpl->_subgMap[subgraph_.id],
         const_cast<char*>(id.c_str()),
         1);
  
  Node node;
  node.id = id;
  setAttribute(node, "fontsize", "11");
  
  return node;
}

Graph::Edge Graph::addEdge(Node from_, Node to_)
{
  std::string id = generateId();
  
  _graphPimpl->_edgeMap[id]
    = agedge(_graphPimpl->_graph,
             agnode(_graphPimpl->_graph, const_cast<char*>(from_.id.c_str()), 0),
             agnode(_graphPimpl->_graph, const_cast<char*>(to_.id.c_str()), 0),
             const_cast<char*>(id.c_str()),
             1);
  
  Edge edge;
  edge.id = id;
  setAttribute(edge, "fontsize", "11");
  
  return edge;
}

Graph::Subgraph Graph::addSubgraph(std::string id_)
{
  std::string id = id_.empty() ? generateId() : id_;
  
  _graphPimpl->_subgMap[id]
    = agsubg(_graphPimpl->_graph, const_cast<char*>(id.c_str()), 1);
  
  return {id};
}

bool Graph::hasEdge(Node from_, Node to_) const
{
  return
    agedge(_graphPimpl->_graph,
           agnode(_graphPimpl->_graph, const_cast<char*>(from_.id.c_str()), 0),
           agnode(_graphPimpl->_graph, const_cast<char*>(to_.id.c_str()), 0),
           0,
           0);
}

bool Graph::hasNode(Node node_) const
{
  return agnode(_graphPimpl->_graph, const_cast<char*>(node_.id.c_str()), 0);
}

void Graph::delNode(Node node_)
{
  agdelnode(_graphPimpl->_graph,
            agnode(_graphPimpl->_graph, const_cast<char*>(node_.id.c_str()), 0));
}
  
void Graph::delEdge(Node from_, Node to_)
{
  agdeledge(_graphPimpl->_graph,
            agedge(_graphPimpl->_graph,
              agnode(_graphPimpl->_graph, const_cast<char*>(from_.id.c_str()), 0),
              agnode(_graphPimpl->_graph, const_cast<char*>(to_.id.c_str()), 0),
              0,
              0)
            );
}

void Graph::setAttribute(const std::string& key_, const std::string& value_)
{
  agsafeset(_graphPimpl->_graph,
            const_cast<char*>(key_.c_str()),
            const_cast<char*>(value_.c_str()),
            const_cast<char*>(""));
}

void Graph::setAttribute(const Node& node_,
                         const std::string& key_,
                         const std::string& value_)
{
  agsafeset(agnode(_graphPimpl->_graph, const_cast<char*>(node_.id.c_str()), 0),
            const_cast<char*>(key_.c_str()),
            const_cast<char*>(value_.c_str()),
            const_cast<char*>(""));
}

void Graph::setAttribute(const Edge& edge_,
                         const std::string& key_,
                         const std::string& value_)
{
  agsafeset(_graphPimpl->_edgeMap[edge_.id],
            const_cast<char*>(key_.c_str()),
            const_cast<char*>(value_.c_str()),
            const_cast<char*>(""));
}

void Graph::setAttribute(const Subgraph& graph_,
                         const std::string& key_,
                         const std::string& value_)
{
  agsafeset(_graphPimpl->_subgMap[graph_.id],
            const_cast<char*>(key_.c_str()),
            const_cast<char*>(value_.c_str()),
            const_cast<char*>(""));
}

void Graph::setAttribute(const Node& targetNode_,
                         const Node& sourceNode_)
{
  agcopyattr(agnode(_graphPimpl->_graph, const_cast<char*>(sourceNode_.id.c_str()), 0),
             agnode(_graphPimpl->_graph, const_cast<char*>(targetNode_.id.c_str()), 0));
}
  
void Graph::setAttribute(const Edge& targetEdge_,
                         const Edge& sourceEdge_)
{
  agcopyattr(_graphPimpl->_edgeMap[sourceEdge_.id],
             _graphPimpl->_edgeMap[targetEdge_.id]);
}

std::string Graph::getAttribute(const Node& node_,
                         std::string key_)
{
  auto ret =
    agget(agnode(_graphPimpl->_graph, const_cast<char*>(node_.id.c_str()), 0),
      const_cast<char*>(key_.c_str()));

  return ret ? ret : "";
}


std::string Graph::getAttribute(const Edge& edge_,
                         std::string key_)
{
  auto ret =
    agget(_graphPimpl->_edgeMap[edge_.id], const_cast<char*>(key_.c_str()));

  return ret ? ret : "";
}

// TODO: layout algoritmus
// TODO: ez majdnem ugyanaz, mint a dotToSvg -> ki kéne emelni
// TODO: kétszer egymás után hvva segfaultot ad
std::string Graph::output(Graph::Format format_) const
{
  char** result        = new char*;
  unsigned int* length = new unsigned int;
  
  gvLayout(_graphPimpl->_gvc, _graphPimpl->_graph, "dot");
  
  gvRenderData(_graphPimpl->_gvc,
               _graphPimpl->_graph,
               format_ == Graph::DOT ? "dot" : "svg",
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
    = agnode(_graphPimpl->_graph, const_cast<char*>(node.id.c_str()), 0);
  
  if (!n)
    return result;
  
  for (Agedge_t* edge = agfstout(_graphPimpl->_graph, n);
       edge;
       edge = agnxtout(_graphPimpl->_graph, edge))
    result.push_back({std::string(agnameof(aghead(edge)))});
  
  return result;
}

std::vector<Graph::Node> Graph::getParents(const Node& node) const
{
  std::vector<Graph::Node> result;
  
  Agnode_t* n
    = agnode(_graphPimpl->_graph, const_cast<char*>(node.id.c_str()), 0);
  
  if (!n)
    return result;
  
  for (Agedge_t* edge = agfstin(_graphPimpl->_graph, n);
       edge;
       edge = agnxtin(_graphPimpl->_graph, edge))
    result.push_back({std::string(agnameof(agtail(edge)))});
  
  return result;
}

std::string Graph::generateId()
{
  size_t i;
  for (i = 0; i < currentId.size() && currentId[i] == 'z'; ++i)
    currentId[i] = 'a';
  
  if (i == currentId.size())
    currentId.push_back('a');
  else
    ++currentId[i];
  
  return currentId;
}

bool operator<(const Graph::Node& n1, const Graph::Node& n2)
{
  return n1.id < n2.id;
}

bool operator<(const Graph::Edge& e1, const Graph::Edge& e2)
{
  return e1.id < e2.id;
}

bool operator<(const Graph::Subgraph& s1, const Graph::Subgraph& s2)
{
  return s1.id < s2.id;
}

bool operator==(const Graph::Node& n1, const Graph::Node& n2)
{
  return n1.id == n2.id;
}

bool operator==(const Graph::Edge& e1, const Graph::Edge& e2)
{
  return e1.id == e2.id;
}

bool operator==(const Graph::Subgraph& s1, const Graph::Subgraph& s2)
{
  return s1.id == s2.id;
}

} // util
} // cc

