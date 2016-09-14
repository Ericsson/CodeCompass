#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "externalusers.h"

namespace cc {
namespace service {
namespace diagram {

void ExternalUsers::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);
  
  std::unordered_set<model::FileId> descendants = util::diagram::bfs(
    decorator,
    startNode,
    relations.subdir,
    CppEdgeType::SUBDIR);
  
  descendants.insert(startNode);
  
  for (const model::FileId& subdir : descendants)
  {
    for (const model::FileId& implementModule : relations.implementedby(subdir))
      if (descendants.find(implementModule) == descendants.end())
        decorator.addEdge(implementModule, subdir, CppEdgeType::IMPLEMENTEDBY);
    for (const model::FileId& dependModule : relations.dependenton(subdir))
      if (descendants.find(dependModule) == descendants.end())
        decorator.addEdge(dependModule, subdir, CppEdgeType::DEPENDENTON);
  }
  
  decorator.decorateNodes();
  decorator.doAdditionalEdgeDecoration();
}

}
}
}

