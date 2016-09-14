#ifndef UTIL_GRAPHBUILDER_H
#define	UTIL_GRAPHBUILDER_H

#include <cstddef>
#include <vector>
#include <list>
#include <queue>
#include <unordered_map>
#include <utility>
#include <functional>
#include <algorithm>
#include <ctime>
#include "util.h"

namespace cc 
{
namespace util 
{
    
struct Empty
{
  bool operator==(const Empty &state_)
  {
    return true;
  }
};

template <typename G, typename T, typename S = Empty>
class GraphBuilder
{
  typedef typename G::Node GNode;
  typedef typename G::Edge GEdge;
  
  typedef std::pair<T, T> TPair;
  typedef std::pair<T, S> TSPair;
  typedef std::vector<TSPair> TSVector;
  
public:
  struct Rule
  {
    Rule()
      : isNode(nullptr),
        nodeAction(nullptr),
        edgeAction(nullptr),
        children(nullptr),
        parents(nullptr),
        childSequence(nullptr),
        childSequences(nullptr),
        inNodes(nullptr),
        outNodes(nullptr),
        furtherEdges(nullptr)
    { }
    
    std::function<bool(T, S)>                   isNode;
    std::function<void(T, S, GNode)>            nodeAction;
    std::function<void(T, S, T, S, GEdge)>      edgeAction;
    std::function<TSVector(T, S)>               children, parents;
    std::function<TSVector(T, S)>               childSequence;
    std::function<std::vector<TSVector>(T, S)>  childSequences;    
    std::function<TSVector(T, S)>               inNodes;
    std::function<std::vector<T>(T, S)>         outNodes;
    std::function<
      std::vector<
        std::pair<TSPair, TSPair>>(T, S)>       furtherEdges;
  };

private:
  struct Node
  {
  private:
    GNode _node;
    
  public:
    std::list<S> states;
    
    Node(GNode node_) : _node(node_) { }
    GNode node() const { return _node; }
  };
  
  struct Edge
  {
  private:
    GNode _source, _target;
    GEdge _edge;
    
  public:    
    Edge(GNode source_, GNode target_, GEdge edge_) 
      : _source(source_), _target(target_), _edge(edge_) { }
    
    GNode source() const { return _source; }
    GNode target() const { return _target; }
    GEdge edge()   const { return _edge;   }
  };
  
public:  
  virtual ~GraphBuilder() { }
  void addRule(const Rule& rule_);
  bool buildGraph(G& graph_, const T& startValue_, const S& startState_);
  
private:
  GNode addNode(const TSPair& pair_);
  GNode addNode(const T& value_, const S& state_);
  GEdge addEdge(T sourceValue_, GNode source_,
                      T targetValue_, GNode target_);
  
  G*                 _graph;
  std::vector<Rule>  _rules;
  std::queue<TSPair> _queue;
  
  std::unordered_map<T, Node>        _nodeMap;
  std::unordered_map<TPair, Edge>    _edgeMap;
  std::unordered_multimap<T, TSPair> _todoIn;
  std::unordered_multimap<T, TSPair> _todoOut;
};

template <typename G, typename T, typename S>
void GraphBuilder<G, T, S>::addRule(const Rule& rule_)
{
  if (rule_.isNode)
    _rules.push_back(rule_);
}

template <typename G, typename T, typename S>
bool GraphBuilder<G, T, S>::buildGraph(G& graph_, const T& startValue_, const S& startState_)
{
  _graph = &graph_;
  _nodeMap.clear();
  _edgeMap.clear();
  _todoIn.clear(); 
  _todoOut.clear(); 
  
  try {
    addNode(startValue_, startState_);
    while (!_queue.empty())
    {
      T currentValue   = _queue.front().first;
      S currentState   = _queue.front().second;
      Node currentNode = _nodeMap.at(currentValue);
      _queue.pop();

      for (const Rule& rule : _rules)
      {
        if (rule.nodeAction)
          rule.nodeAction(currentValue, currentState, currentNode.node());

        if (!rule.isNode(currentValue, currentState))
          continue;

        if (rule.children)
          for (TSPair childTS : rule.children(currentValue, currentState))
          {
            GNode gChild = addNode(childTS);
            GEdge gEdge  = addEdge(currentValue, currentNode.node(),
                                   childTS.first, gChild);
            if (rule.edgeAction)
              rule.edgeAction(currentValue, currentState,
                              childTS.first, childTS.second,
                              gEdge);
          }

        if (rule.parents)
          for (TSPair parentTS : rule.parents(currentValue, currentState))
          {
            GNode gParent = addNode(parentTS);
            GEdge gEdge = addEdge(parentTS.first, gParent,
                                  currentValue, currentNode.node());
            if (rule.edgeAction)
              rule.edgeAction(parentTS.first, parentTS.second,
                              currentValue, currentState,
                              gEdge);
          }

        std::vector<TSVector> sequences;
        if (rule.childSequences)
          sequences = rule.childSequences(currentValue, currentState);
        if (rule.childSequence)
          sequences.push_back(rule.childSequence(currentValue, currentState));

        for (const TSVector& sequence : sequences)
        {
          const TSPair *prevTS = nullptr;

          if (!sequence.empty())
          {
            GEdge gEdge = addEdge(currentValue, currentNode.node(),
                                  sequence[0].first, addNode(sequence[0]));
            if (rule.edgeAction)
              rule.edgeAction(currentValue, currentState,
                              sequence[0].first, sequence[0].second,
                              gEdge);

            prevTS = &sequence[0];
          }

          for (std::size_t i = 1; i < sequence.size(); ++i)
          {
            addNode(sequence[i]);
            _todoOut.insert(std::make_pair(prevTS->first, sequence[i]));
            prevTS = &sequence[i];
          }
        }

        typename std::unordered_multimap<T, TSPair>::iterator item;
        while ((item = _todoOut.find(currentValue)) != _todoOut.end())
        {
          if (rule.outNodes)
            for (T outValue : rule.outNodes(currentValue, currentState))
              _todoOut.insert(std::make_pair(outValue, item->second));
          else
            addEdge(currentValue, currentNode.node(),
                    item->second.first, addNode(item->second));

          _todoOut.erase(item);
        }

        // This part of the code probably segfaults
        // Requires an overview and a proof-checking
        if (rule.inNodes)
          for (TSPair inTS : rule.inNodes(currentValue, currentState))
          {
            _todoIn.insert(std::make_pair(currentValue, inTS));

            for(const std::pair<TPair, Edge>& edge : _edgeMap)
              if(edge.first.second == currentValue)
              {
                GNode gTarget = addNode(inTS);
                GEdge gEdge   = _graph->addEdge(edge.second.source(), gTarget);

                _edgeMap.insert(std::make_pair(std::make_pair(edge.first.first, inTS.first),
                                               Edge(edge.second.source(), gTarget, gEdge)));

                if (rule.edgeAction)
                  rule.edgeAction(edge.first.first, currentState,
                                  inTS.first, inTS.second,
                                  gEdge);

                _graph->delEdge(edge.second.source(), edge.second.target());
                _edgeMap.erase(edge.first);
              }
          }

        if (rule.furtherEdges)
          for (std::pair<TSPair, TSPair> edge : rule.furtherEdges(currentValue, currentState))
          {
            GNode source = addNode(edge.first), target = addNode(edge.second);
            GEdge gEdge  = _graph->addEdge(source, target);
            _edgeMap.insert(std::make_pair(std::make_pair(edge.first.first, edge.second.first),
                                           Edge(source, target, gEdge)));

            if (rule.edgeAction)
              rule.edgeAction(edge.first.first, edge.first.second,
                              edge.second.first, edge.second.second,
                              gEdge);
          }
      }

      typename std::unordered_multimap<T, TSPair>::iterator item;
      while ((item = _todoOut.find(currentValue)) != _todoOut.end())
      {
        addEdge(currentValue, currentNode.node(),
                item->second.first, addNode(item->second));
        _todoOut.erase(item);
      }
    }
  }
  catch (TooLongException ex) {
    std::cout << ex.what() << std::endl;
    return false;
  }
  
  return true; 
}

template <typename G, typename T, typename S>
typename G::Node GraphBuilder<G, T, S>::addNode(const TSPair& pair_)
{
  return addNode(pair_.first, pair_.second);
}

template <typename G, typename T, typename S>
typename G::Node GraphBuilder<G, T, S>::addNode(const T& value_, const S& state_)
{
  GNode gNode;
  auto nodeIt = _nodeMap.find(value_);
  if (nodeIt == _nodeMap.end())
  {
    gNode = _graph->addNode();
    Node node(gNode);
    
    _queue.push(std::make_pair(value_, state_));
    node.states.push_back(state_);
    _nodeMap.insert(std::make_pair(value_, node));
  }
  else
  {
    gNode = nodeIt->second.node();
    Node& node = _nodeMap.at(value_);
    
    auto stateIt = std::find(node.states.begin(), node.states.end(), state_);
    if(stateIt == node.states.end())
    {
      _queue.push(std::make_pair(value_, state_));
      node.states.push_back(state_);
    }
  }
  return gNode;
}

template <typename G, typename T, typename S>
typename G::Edge GraphBuilder<G, T, S>::addEdge(T sourceValue_, GNode source_,
                                        T targetValue_, GNode target_)
{
  GEdge gEdge;
  auto inNodeIt = _todoIn.find(targetValue_);
  if (inNodeIt == _todoIn.end())
  {
    auto edgeIt = _edgeMap.find(std::make_pair(sourceValue_, targetValue_));
    if(edgeIt == _edgeMap.end())
    {
      gEdge = _graph->addEdge(source_, target_);
      _edgeMap.insert(std::make_pair(std::make_pair(sourceValue_, targetValue_),
                                     Edge(source_, target_, gEdge)));
    }
    else
      gEdge = edgeIt->second.edge();
  }
  else
  {
    typedef typename std::unordered_multimap<T, TSPair>::iterator todoIt;
    
    std::pair<todoIt, todoIt> range = _todoIn.equal_range(targetValue_);
    for (auto it = range.first; it != range.second; ++it)
    {
      auto edgeIt = _edgeMap.find(std::make_pair(sourceValue_, it->second.first));
      if(edgeIt == _edgeMap.end())
      {
        GNode gTarget = addNode(it->second);
        gEdge = _graph->addEdge(source_, gTarget);
        _edgeMap.insert(std::make_pair(std::make_pair(sourceValue_, it->second.first),
                                       Edge(source_, gTarget, gEdge)));
      }
      else
        gEdge = edgeIt->second.edge();
    }
  }
  return gEdge;
}

} // util
} // cc

namespace std
{
  template <typename T>
  struct hash<std::pair<T, T>>
  {
    size_t operator()(const std::pair<T, T> &pair_) const
    {
      return hash<T>()(pair_.first) ^ hash<T>()(pair_.second);
    }
  };
} // std

#endif
