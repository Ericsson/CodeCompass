#ifndef CC_UTIL_DIAGRAM_BFS_H
#define CC_UTIL_DIAGRAM_BFS_H

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_set>
#include <functional>

#include <odb/database.hxx>

#include <util/graph/graph.h>
#include <util/diagram/graphdecorator.h>

namespace cc {
namespace util {
namespace diagram {

/**
 * This function builds a graph in the order of breadth-first search.
 * @param decorator A decorator object \see{cc:util::diagram::GraphDecorator}.
 * @param startNode Bredth-first search starts from this node. This node is not
 * inserted into the returning set unless there is a loop in the graph which
 * contains this node.
 * @param relations These functions describe the relations which determine the
 * edges of the graph.
 * @param edgeTypes These are needed to pass to the decorator. The size of this
 * vector must be equal to the size of the vector containing relations.
 * @param pred The graph will contain the nodes on which this predicate function
 * returns true.
 * @return The returning set contains the visited nodes except the startNode
 * \see{startNode}.
 * @pre The length of relations and edgeTypes must be equal.
 */
template<typename NodeType, typename EdgeType, typename Relation>
std::unordered_set<NodeType> bfs(
  GraphDecorator<NodeType, EdgeType>& decorator,
  const NodeType& startNode,
  std::vector<Relation> relations,
  std::vector<EdgeType> edgeTypes,
  std::function<bool(const NodeType&)> pred = [](const NodeType&){ return true; })
{
  assert(relations.size() == edgeTypes.size());
  
  std::unordered_set<NodeType> visitedNodes;

  std::queue<NodeType> queue;
  queue.push(startNode);

  while (!queue.empty())
  {
    NodeType current = queue.front();
    queue.pop();

    for (std::size_t i = 0; i < relations.size(); ++i)
      for (const NodeType& to : relations[i](current))
        if (pred(to))
        {
          decorator.addEdge(current, to, edgeTypes.at(i));
          if (visitedNodes.find(to) == visitedNodes.end())
          {
            queue.push(to);
            visitedNodes.insert(to);
          }
        }
  }

  return visitedNodes;
}

/**
 * This function builds a graph in the order of breadth-first search.
 * @param decorator A decorator object \see{cc:util::diagram::GraphDecorator}.
 * @param startNode Bredth-first search starts from this node. This node is not
 * inserted into the returning set unless there is a loop in the graph which
 * contains this node.
 * @param relation This function describes the relation which determines the
 * edges of the graph.
 * @param edgeType This is needed to pass to the decorator.
 * @param pred The graph will contain the nodes on which this predicate function
 * returns true.
 * @return The returning set contains the visited nodes except the startNode
 * \see{startNode}.
 */
template<typename NodeType, typename EdgeType>
std::unordered_set<NodeType> bfs(
  GraphDecorator<NodeType, EdgeType>& decorator,
  const NodeType& startNode,
  std::function<std::vector<NodeType>(const NodeType&)> relation,
  EdgeType edgeType,
  std::function<bool(const NodeType&)> pred = [](const NodeType&){ return true; })
{
  return bfs<NodeType, EdgeType, std::function<std::vector<NodeType>(const NodeType&)>>(
    decorator, startNode, {relation}, {edgeType}, pred);
}

}
}
}

#endif // CC_UTIL_DIAGRAM_BFS_H
