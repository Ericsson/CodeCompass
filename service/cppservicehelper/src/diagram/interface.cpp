#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "interface.h"

namespace cc {
namespace service {
namespace diagram {

void InterfaceDiagram::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);

  for (const model::FileId& fileId : relations.provides(startNode))
    decorator.addEdge(startNode, fileId, CppEdgeType::PROVIDES);

  for (const model::FileId& fileId : relations.contains(startNode))
    decorator.addEdge(startNode, fileId, CppEdgeType::CONTAINS);

  for (const model::FileId& fileId : relations.uses(startNode))
    decorator.addEdge(startNode, fileId, CppEdgeType::USES);

  for (const model::FileId& fileId : relations.providedby(startNode))
    decorator.addEdge(fileId, startNode, CppEdgeType::PROVIDEDBY, true);

  for (const model::FileId& fileId : relations.containedby(startNode))
    decorator.addEdge(fileId, startNode, CppEdgeType::CONTAINEDBY, true);

  for (const model::FileId& fileId : relations.usedby(startNode))
    decorator.addEdge(fileId, startNode, CppEdgeType::USEDBY, true);

  //--- Decorate nodes ---//

  decorator.decorateNodes();
  decorator.doAdditionalEdgeDecoration();
}

}
}
}
