#include <llvm/Support/raw_os_ostream.h>
#include <clang/AST/Decl.h>

#include "pdg.h"
#include "pdg_builder.h"

namespace cc
{ 
namespace service
{  
namespace language
{

void PDG::build(clang::Decl* decl, clang::SourceManager* sm)
{
  PDG_builder builder(db, *this, sm);    
  builder.TraverseDecl(decl);
  
  createExit();
  
  builder.add_data_edges();
}

void PDG::dump2dot(std::ostream& os)
{
  os << "digraph pdg {" << std::endl;
  
  Node& n = node(entry());
  os << n.id << "[ label = \"" << n.id << " - " << n.label << "\" ]" << std::endl;
  dumpChildren(os, n.id);
  
  for( unsigned int i=0; i<graph.size(); ++i )
  {    
    for(Edge& e : graph[i])
    {
      os << i << " -> " << e.first;
      if(e.second == "back control")
        os << "[ color=blue ]";
      else if(e.second == "data")
        os << "[ color=green ]";
      os << std::endl;
    }
  }    
  os << "}" << std::endl;
  
  os << "def - used for nodes: " << std::endl;
  for(Node& n : nodes)
  {
    os << n.id << " -- defs: ";
    for(auto& x : n.defs)
      os << x << ", ";
    os << std::endl;
    
    os << n.id << " -- uses: ";
    for(auto& x : n.uses)
      os << x << ", ";
    os << std::endl;
  }
}

void PDG::dumpChildren(std::ostream& os, Node::id_t n)
{
  for(Node::id_t child : children(n))
  {
    os << child << "[ label = \"" << child << " - " << node(child).label << "\" ]" << std::endl;
  }
  
  os << "{ rank=same; ";
  for(Node::id_t child : children(n))
  {
    os << child << " ";
  } 
  os << "}" << std::endl;
  
  for(Node::id_t child : children(n))
  {
    dumpChildren(os, child);
  }
  
}

Node::id_t PDG::nodeIdFromPos(const core::FilePosition& filepos)
{  
   //std::cerr << "DEBUG: Looking for nodes (size): " << nodes.size() << std::endl;  
  //std::cerr << "DEBUG: in position: " << filepos.pos.line << ", " << filepos.pos.column << std::endl;
  for(const Node& node : nodes)
  {
    //std::cerr << "DEBUG: pos" << node.range.startpos.line << " -- " << node.range.endpos.line << std::endl;
    if(node.range.startpos.line <= filepos.pos.line && node.range.startpos.column <= filepos.pos.column &&
       node.range.endpos.line >= filepos.pos.line)
      return node.id;
  }
  return Node::invalid_id;
}

void PDG::backwardSlicePositions(std::vector<core::Range>& result,  const core::FilePosition& filePos)
{
  std::set<core::Range, CompareRange> res;
  backwardSlicePositions(res, nodeIdFromPos(filePos));
  result.insert(result.end(), res.begin(), res.end());
}

void PDG::backwardSlicePositions(std::set<core::Range, CompareRange>& result,  Node::id_t node_id)
{ 
  if(node_id == Node::invalid_id)
  {
    std::cerr << "No node selected" << std::endl;
    return;
  }
  //std::cerr << "DEBUG: start backward slice from node_id: " << node_id << std::endl;
  std::set<Node::id_t> to_visit{node_id};  
  std::set<Node::id_t> visited;  
  
  //std::cerr << "visiting node: ";
  while(!to_visit.empty())
  {
    Node& n = node(*to_visit.begin());    
    //std::cerr << n.id << " ";
    visited.insert(n);
    to_visit.erase(to_visit.begin());
    
    result.insert(n.range);
    if(parents[n] != 0 && visited.count(parents[n]) == 0)
      to_visit.insert(parents[n]);
    
    for( Node::id_t k : data_parents[n])
      if( visited.count(k) == 0 )
        to_visit.insert(k);
  }  
}

void PDG::forwardSlicePositions(std::vector<core::Range>& result, const core::FilePosition& filePos)
{
  std::set<core::Range, CompareRange> res;
  forwardSlicePositions(res, nodeIdFromPos(filePos));
  result.insert(result.end(), res.begin(), res.end());
}

void PDG::forwardSlicePositions(std::set<core::Range, CompareRange>& result, Node::id_t node_id)
{
  if(node_id == Node::invalid_id)
  {
    std::cerr << "No node selected" << std::endl;
    return;
  }
  
  std::set<Node::id_t> to_visit{node_id};  
  std::set<Node::id_t> visited;
  
  while(!to_visit.empty())
  {
    Node& n = node(*to_visit.begin());    
    //std::cerr << n.id << " ";
    visited.insert(n);
    to_visit.erase(to_visit.begin());
    
    result.insert(n.range);
    
    for( Node::id_t child : dataChildren(n))
      if( visited.count(child) == 0 )
        to_visit.insert(child);        
    
    for( Node::id_t child : children(n))
      if( visited.count(child) == 0 )
        to_visit.insert(child);
  }  
}

} // language
} // service
} // cc
