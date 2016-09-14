#ifndef GRAPH_DECORATOR_H
#define GRAPH_DECORATOR_H

#include <algorithm>
#include <memory>
#include <map>
#include <unordered_map>
#include <util/odbtransaction.h>
#include <util/graph.h>
#include <odb/database.hxx>
#include "model/file.h"
#include "model/file-odb.hxx"

namespace cc {
namespace util {
namespace diagram {

template <typename NodeType, typename EdgeType>
class GraphDecorator
{
public:
  GraphDecorator(util::Graph& graph) : graph(graph) {}
  virtual ~GraphDecorator() {}

  virtual std::map<std::string, std::string> decorateNode(const NodeType&)
  {
    return std::map<std::string, std::string>();
  }
  
  virtual std::map<std::string, std::string> decorateEdge(const EdgeType&)
  {
    return std::map<std::string, std::string>();
  }
  
  void setReversePairs(const std::set<std::pair<EdgeType, EdgeType>>& pairs)
  {
    reversePairs = pairs;
  }
  
  bool isReversePairs(const EdgeType& edge1, const EdgeType& edge2) const
  {
    return reversePairs.find({edge1, edge2}) != reversePairs.end()
        || reversePairs.find({edge2, edge1}) != reversePairs.end();
  }
  
  void addEdge(
    const NodeType& from,
    const NodeType& to,
    EdgeType edgeType,
    bool reverseEdge = false)
  {
    typename NodeMap::const_iterator fromIt = visitedNodes.find(from);
    typename NodeMap::const_iterator toIt   = visitedNodes.find(to);

    util::Graph::Node fromFile, toFile;

    if (fromIt == visitedNodes.end())
    {
      fromFile = graph.addNode();
      visitedNodes[from] = fromFile;
      revNodeMap[fromFile] = from;
    }
    else
      fromFile = fromIt->second;

    if (toIt == visitedNodes.end())
    {
      toFile = graph.addNode();
      visitedNodes[to] = toFile;
      revNodeMap[toFile] = to;
    }
    else
      toFile = toIt->second;

    if (isEdgeNeeded(from, to, edgeType))
    {
      util::Graph::Edge edge = graph.addEdge(fromFile, toFile);

      if (reverseEdge)
        graph.setAttribute(edge, "dir", "back");

      for (const auto& decoration : decorateEdge(edgeType))
        graph.setAttribute(edge, decoration.first, decoration.second);
      
      edges.insert({{from, to}, {edgeType, edge}});
    }
  }

  void delNode(const NodeType& fileId)
  {
    graph.delNode(visitedNodes[fileId]);
    visitedNodes.erase(fileId);
    
    for (auto it = edges.cbegin(); it != edges.cend();)
      if (it->first.first == fileId || it->first.second == fileId)
        it = edges.erase(it);
      else
        ++it;
  }
  
  void decorateNodes()
  {
    for (const auto& node : visitedNodes)
      for (const auto& decoration : decorateNode(node.first))
        graph.setAttribute(node.second, decoration.first, decoration.second);
  }

  const NodeType& getGraphNode(const util::Graph::Node& node) const
  {
    return revNodeMap.find(node)->second;
  }
  
  const util::Graph::Node& getNode(const NodeType& node) const
  {
    return visitedNodes.find(node)->second;
  }
  
  void unlanceNode(const NodeType& node, bool keepIncoming)
  {
    if (visitedNodes.find(node) == visitedNodes.end())
      return;
    
    Edges todoEdges;
    
    util::Graph::Node graphNode = visitedNodes[node];
    
    std::vector<util::Graph::Node> children = graph.getChildren(graphNode);
    std::vector<util::Graph::Node> parents  = graph.getParents(graphNode);
    
    if (keepIncoming)
    {
      for (const util::Graph::Node& parent : parents)
      {
        const NodeType& ntParent = revNodeMap[parent];
        
        typename Edges::iterator it;
        while ((it = edges.find({ntParent, node})) != edges.end())
        {
          for (const util::Graph::Node& child : children)
          {
            const NodeType& ntChild = revNodeMap[child];
            if (ntParent != node && ntChild != node)
              todoEdges.insert({{ntParent, ntChild}, it->second});
          }
          edges.erase(it);
//          graph.delEdge(parent, graphNode);
        }
      }
      
      for (const util::Graph::Node& child : children)
      {
        edges.erase({node, revNodeMap[child]});
//        graph.delEdge(graphNode, child);
      }
    }
    else
    {
      for (const util::Graph::Node& child : children)
      {
        const NodeType& ntChild = revNodeMap[child];
        
        typename Edges::iterator it;
        while ((it = edges.find({node, ntChild})) != edges.end())
        {
          for (const util::Graph::Node& parent : parents)
          {
            const NodeType& ntParent = revNodeMap[parent];
            if (ntChild != node && ntParent != node)
              todoEdges.insert({{ntParent, ntChild}, it->second});
          }
          edges.erase(it);
//          graph.delEdge(graphNode, child);
        }
      }
      
      for (const util::Graph::Node& parent : parents)
      {
        edges.erase({revNodeMap[parent], node});
//        graph.delEdge(parent, graphNode);
      }
    }
    
    graph.delNode(graphNode);
    visitedNodes.erase(node);
    revNodeMap.erase(graphNode);
    
    for (typename Edges::const_iterator i = todoEdges.begin();
         i != todoEdges.end();
         ++i)
      addEdge(i->first.first, i->first.second, i->second.edgeType);
  }
  
protected:
  util::Graph& graph;
  
  struct EdgeInfo
  {
    EdgeType edgeType;
    util::Graph::Edge edge;
  };
  
  typedef std::unordered_map<NodeType, util::Graph::Node> NodeMap;
  NodeMap visitedNodes;
  
  typedef std::map<util::Graph::Node, NodeType> RevNodeMap;
  RevNodeMap revNodeMap;

  typedef std::multimap<std::pair<NodeType, NodeType>, EdgeInfo> Edges;
  Edges edges;
  
private:
  std::set<std::pair<EdgeType, EdgeType>> reversePairs;

  bool isEdgeNeeded(
    const NodeType& from, const NodeType& to, const EdgeType& type) const
  {
    auto f = [&, this](const typename Edges::value_type& p) {
      return p.second.edgeType == type ||
             isReversePairs(p.second.edgeType, type);
    };
    
    std::pair<
      typename Edges::const_iterator,
      typename Edges::const_iterator>
    range = edges.equal_range({from, to});
    
    if (std::find_if(range.first, range.second, f) != range.second)
      return false;
    
    range = edges.equal_range({to, from});
    
    if (std::find_if(range.first, range.second, f) != range.second)
      return false;
    
    return true;
  }
};

}
}
}

#endif
