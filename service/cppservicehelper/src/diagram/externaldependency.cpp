#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "externaldependency.h"

namespace cc {
namespace service {
namespace diagram {

void ExternalDependency::buildDiagram(
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
    for (const model::FileId& implementModule : relations.implements(subdir))
      if (descendants.find(implementModule) == descendants.end())
        decorator.addEdge(subdir, implementModule, CppEdgeType::IMPLEMENTS);
    for (const model::FileId& dependModule : relations.depends(subdir))
      if (descendants.find(dependModule) == descendants.end())
        decorator.addEdge(subdir, dependModule, CppEdgeType::DEPENDS);
  }
  
  decorator.decorateNodes();
  decorator.doAdditionalEdgeDecoration();
}

}
}
}
