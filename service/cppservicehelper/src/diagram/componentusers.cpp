#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>

#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "componentusers.h"

namespace cc {
namespace service {
namespace diagram {

void ComponentUsers::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);

  //--- Provided interfaces ---//

  std::unordered_set<model::FileId> usedHeaders;
  std::unordered_set<model::FileId> interfaces;
  
  for (const model::FileId& header : relations.provides(startNode))
  {
    decorator.addEdge(startNode, header, CppEdgeType::PROVIDES);
    interfaces.insert(header);
    usedHeaders.insert(header);
  }

  //--- Interface users ---//

  for (const model::FileId& header : interfaces)
  {
    bool needed = false;

    std::queue<model::FileId> queue;
    queue.push(header);

    while (!queue.empty())
    {
      model::FileId current = queue.front();
      queue.pop();

      for (const model::FileId& user : relations.usedby(current))
      {
        decorator.addEdge(current, user, CppEdgeType::USEDBY);
        if (usedHeaders.find(user) == usedHeaders.end())
        {
          queue.push(user);
          usedHeaders.insert(user);
        }
      }

      needed = true;
    }

    if (!needed)
      decorator.delNode(header);
  }

  //--- Container binaries ---//

  for (const model::FileId& source : usedHeaders)
    bfs(
      decorator,
      source,
      relations.containedby,
      CppEdgeType::CONTAINEDBY);

  //--- Decorate nodes ---//

  decorator.decorateNodes();
  decorator.doAdditionalEdgeDecoration();
}

}
}
}
