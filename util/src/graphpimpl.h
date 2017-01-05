#ifndef UTIL_GRAPHPIMPL_H
#define	UTIL_GRAPHPIMPL_H

#include <graphviz/gvc.h>
#include <map>
#include <string>

namespace cc
{
namespace util
{

struct GraphPimpl
{
  GraphPimpl(std::string name_     = "",
             bool        directed_ = true,
             bool        strict_   = false)
  {
    Agdesc_t type;
  
    if (strict_)
      if (directed_) type = Agstrictdirected;
      else           type = Agstrictundirected;
    else
      if (directed_) type = Agdirected;
      else           type = Agundirected;

    _gvc   = gvContext();
    _graph = agopen(const_cast<char*>(name_.c_str()), type, 0);
  }
  
  ~GraphPimpl()
  {
    agclose(_graph);
    gvFreeContext(_gvc);
    
    _graph = 0;
    _gvc   = 0;
  }
  
  Agraph_t* _graph;
  GVC_t*    _gvc;
  
  // These maps are needed, because it isn't possible to get an edge and //
  // subgraph of the graph by name, using the own API of Graphviz.       //
  std::unordered_map<std::string, Agedge_t*> _edgeMap;
  std::unordered_map<std::string, Agraph_t*> _subgMap;
  
private:
  GraphPimpl(const GraphPimpl&);
  GraphPimpl& operator=(const GraphPimpl&);
};

} // util
} // cc

#endif
