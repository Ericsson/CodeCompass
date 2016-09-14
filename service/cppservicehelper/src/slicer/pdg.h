#ifndef SERVICE_CPPSERVICEHELPER_SLICER_PDG_H
#define SERVICE_CPPSERVICEHELPER_SLICER_PDG_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <iostream>

#include <model/cxx/cppastnode.h>
#include <core-api/common_types.h>

namespace clang
{
  class Decl;
  class SourceManager;
}

namespace cc
{ 
namespace service
{  
namespace language
{

  
/**
 * Helper class to allow us to put ranges in std::set. 
 */
struct CompareRange
{
  bool operator()(const core::Range& a, const core::Range& b) const
  {
    if( a.startpos.line != b.startpos.line )
      return a.startpos.line < b.startpos.line;
    else if( a.startpos.column != b.startpos.column )
      return a.startpos.column < b.startpos.column;
    else if( a.endpos.line != b.endpos.line )
      return a.endpos.line < b.endpos.line;
    else
      return a.endpos.column < b.endpos.column;
  }
};
  
/**
 * Represent a node in a Program Dependency Graph (PDG). We distiguish between five node type:
 * Entry, Exit, Predicate (belongs to any decision point in a program, such as condition part of the 
 * if or loop statements), Statement (belongs to other expression statements in a source code), 
 * Region (helper node to make easier to find for example the then or else branch in if statement).
 * 
 * Nodes refer to variables read (uses) or written (defs) to the corresponding statement or predicate.
 */
struct Node
{
  typedef unsigned int id_t;
  static const id_t invalid_id = static_cast<id_t>(-1);
 // static const id_t no_parent = static_cast<unsigned int>(-1);
  enum Type { Region, Predicate, Statement, Entry, Exit };
    
  Type type;          ///< type of the node: Region, Predicate, Statement, Entry, Exit
  void* clangAstPtr;  ///< pointer to a clang AST correspondant node
  core::Range range;  ///< the corresponding clang ast node's position in the source code
  std::string label;  ///< label of the node: such as "if_pred", "for_region" etc.
  id_t id;            ///< we refer the nodes by their id in PDG
    
  std::set<model::HashType> defs; ///< Mangled name hash of the variables that are written in this Node
    
  std::set<model::HashType> uses; ///< Mangled name hash of the variables that are read in this Node
  
  /**
   * helper function to get the id of the node
   */  
  operator id_t () const { return id; }
    
  Node(Type t, void* clangAstPtr, core::Range range_, const std::string& l, int id_) : 
    type(t), clangAstPtr(clangAstPtr), range(range_), label(l), id(id_) {}  
};

/**
 * target node id, label
 */
typedef std::pair<Node::id_t, std::string> Edge;

/**
 * This class represents the Program Dependency Graph (PDG). The first five member functions are the public
 * interface and the rest of the functions are used in implementation reasons. The PDG graph is build on demand.
 * 
 * PDG stores the nodes in a std::vector, and the node's index in a vector will be its id.
 * The graph itself is stored in a vector, where the index refers to the initial node, and its value is 
 * another vector of edges, where the edge is a pair of a target node id, and a label.
 * 
 * We distiguish the parent nodes between data parent and control parent. Control parents are the default,
 * thus we just refer them as parents. Each node except the Entry 
 * has exactly one control parent but they have arbitrary or zero data parents. We distiguish edges between data, 
 * control and back control edges by their label.
 */

class PDG
{
public:  
  PDG(std::shared_ptr<odb::database> db_) : db(db_) {}
  
  /**
   * We build the PDG on demand for only the target functions. Thus the build member function requires
   * a Clang AST node, which represent a function declaraction, and we need the clang SourceManager 
   * to compute the file position of the entities in the AST.
   * 
   * The SlicerHelper class is responsible to look for the proper fucntion declaration AST node 
   * by a file position (e.g.: the UI send us a file id, and a line and col position, where the user
   * clicked in. The SlicerHelper find the corresponding function declaraction for us)
   */
  void build(clang::Decl*, clang::SourceManager*);
  
  /**
   * This is a helper function: just find the corresponding PDG node and invokes the other overloaded 
   * backwardSlicePositions member function. (While the UI send us a file position, this function is 
   * invoked by CppService)
   */  
  void backwardSlicePositions(std::vector<core::Range>&, const core::FilePosition& filePos);
  
  /**
   * Traverse the PDG graph from the node_id upward, via data and control parents.
   * The nodes that this traversal reaches form the backward slice.
   */
  void backwardSlicePositions(std::set<core::Range, CompareRange>& result,  Node::id_t node_id);
  
  /**
   * This is a helper function: just find the corresponding PDG node and invokes the other overloaded 
   * forwardSlicePositions member function. (While the UI send us a file position, this function is 
   * invoked by CppService)
   */
  void forwardSlicePositions(std::vector<core::Range>&, const core::FilePosition& filePos);
  
  /**
   * Traverse the PDG graph from the node_id downward, via data and control children.
   * The nodes that this traversal reaches form the forward slice.
   */
  void forwardSlicePositions(std::set<core::Range, CompareRange>& result, Node::id_t node_id);
  
  /**
   * Returns with the PDG node with the given positions.
   * This function mainly used by backwardSlicePositions and forwardSlicePositions
   */
  Node::id_t nodeIdFromPos(const core::FilePosition& filePos);
  
  Node::id_t entry() { return nodes[entryId]; }
  Node::id_t last_node() { return nodes.back(); }
  Node& node(Node::id_t id) { return nodes[id]; }      
  
  Node::id_t addNode(Node::Type type, const std::string& label = "", 
                     void* clangAstPtr = nullptr, core::Range range = core::Range() )
  {
    //the id will be the index in the nodes vector
    Node n(type, clangAstPtr, range, label, nodes.size());
    nodes.push_back(n);
    parents.push_back(static_cast<unsigned int>(-1));
    data_parents.push_back(std::vector<Node::id_t>());
    graph.push_back(std::vector<Edge>());
    return nodes.back();
  }   
  
  /**
   * Links two node with control edge. This is the default, we do not add any label.
   */
  void linkNode(Node::id_t fromId, Node::id_t targetId, const std::string& label = "")
  {
    graph[fromId].push_back(std::make_pair(targetId, label));
    parents[targetId] = fromId;                       
  }
  
  /**
   * link two node with back control edge.
   */
  void linkBackControlNode(Node::id_t fromId, Node::id_t targetId)
  {
    graph[fromId].push_back(std::make_pair(targetId, std::string("back control")));
  }
  
  /**
   * link two node with data edge.
   */
  void linkDataNode(Node::id_t fromId, Node::id_t targetId)
  {
    graph[fromId].push_back(std::make_pair(targetId, std::string("data")));
    data_parents[targetId].push_back(fromId);                       
  }
  
  std::vector<Edge>& edges(Node::id_t nodeId) { return graph[nodeId]; }  
  Node::id_t parent(Node::id_t nodeId) const { return parents[nodeId]; }  
  
  /**
   * Returns the prior siblings of the given nodes. 
   * This function use only control edges to compute the siblings
   */
  std::vector<Node::id_t> priorSiblings(Node::id_t nodeId) 
  { 
    std::vector<Node::id_t> res;
    for(Edge e: graph[parents[nodeId]])
    {
      if( !(e.second == "back control" || e.second == "data") && e.first < nodeId)
        res.push_back(e.first);
    }
    return res;    
  }
 
  std::vector<Node::id_t> children(Node::id_t nodeId) 
  {
    std::vector<Node::id_t> res;
    childrenAux(nodeId, res, [](const std::string& s){ return !(s == "back control" || s == "data"); });    
    return res;      
  }
  
  std::vector<Node::id_t> dataChildren(Node::id_t nodeId) 
  {
    std::vector<Node::id_t> res;
    childrenAux(nodeId, res, [](const std::string& s){ return s == "data"; });
    return res;      
  }
  
  template<typename Pred>
  void childrenAux(Node::id_t nodeId, std::vector<Node::id_t>& result, Pred p)
  {
    for(Edge e: graph[nodeId])
    {
      if(p(e.second))
        result.push_back(e.first);
    }
  }
  
  
  void dump2dot(std::ostream& os);    
  void dumpChildren(std::ostream& os, Node::id_t node);
  
  void createEntry()
  {
    entryId = addNode(Node::Entry, "Entry");        
  }
  
  void createExit()
  {
    linkNode(entryId, addNode(Node::Exit, "Exit"));
  }
  
private:
  
  std::vector<std::vector<Edge>> graph;
  std::vector<Node> nodes;
  std::vector<Node::id_t> parents;
  std::vector<std::vector<Node::id_t>> data_parents;
  Node::id_t entryId;
  std::shared_ptr<odb::database> db; 
};

} // language
} // service
} // cc


#endif