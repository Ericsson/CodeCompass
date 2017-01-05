#include <unordered_map>
#include <util/util.h>
#include "includedependency.h"
#include "cppgraphdecorator.h"
#include <util/diagram/bfs.h>

namespace cc {
namespace service {
namespace diagram {

void IncludeDependency::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);

  util::diagram::bfs(
    decorator, startNode, relations.includes, CppEdgeType::USES);
  util::diagram::bfs(
    decorator, startNode, relations.included, CppEdgeType::USEDBY);

  decorator.decorateNodes();
}

}
}
}
