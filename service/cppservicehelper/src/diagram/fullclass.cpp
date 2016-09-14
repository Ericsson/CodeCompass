#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>

#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "fullclass.h"

namespace cc {
namespace service {
namespace diagram {

void FullClass::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);
  
  std::unordered_set<model::FileId> descendants = util::diagram::bfs(
    decorator,
    startNode,
    relations.subfiles,
    CppEdgeType::CONTAINS);
  
  descendants.insert(startNode);
  
  for (const model::FileId& subfile : descendants)
  {
    for (const model::FileId& file : relations.subfiles(subfile))
      decorator.addEdge(file, subfile, CppEdgeType::CONTAINS);
  }
  
  decorator.decorateNodes();
  decorator.doAdditionalEdgeDecoration();
}

}
}
}
