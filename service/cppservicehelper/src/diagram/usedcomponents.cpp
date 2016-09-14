#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <iostream>

#include <util/diagram/bfs.h>
#include "cppgraphdecorator.h"
#include "usedcomponents.h"

namespace cc {
namespace service {
namespace diagram {

void UsedComponents::buildDiagram(
  util::Graph& graph, const model::FileId& startNode)
{
  CppGraphDecorator decorator(db, graph);

  //--- Used headers by startNode ---//

  std::unordered_set<model::FileId> usedHeaders
    = util::diagram::bfs(
        decorator,
        startNode,
        relations.uses,
        CppEdgeType::USES);

  //--- Providers of the headers ---//

  std::unordered_set<model::FileId> providerSources;
  std::unordered_set<model::FileId> keptHeaders;
  std::queue<model::FileId> providedHeaders;

  for (const model::FileId& header : usedHeaders)
    for (const model::FileId& provider : relations.providedby(header))
    {
      decorator.addEdge(header, provider, CppEdgeType::PROVIDEDBY);
      providerSources.insert(provider);
      providedHeaders.push(header);
      keptHeaders.insert(header);
      keptHeaders.insert(provider);
    }
  
  //--- Collect headers which have to be kept ---//
  
  while (!providedHeaders.empty())
  {
    model::FileId header = providedHeaders.front();
    providedHeaders.pop();
    
    for (const util::Graph::Node& parent : graph.getParents(decorator.getNode(header)))
    {
      const model::FileId& parentFile = decorator.getGraphNode(parent);
      if (keptHeaders.find(parentFile) == keptHeaders.end())
      {
        keptHeaders.insert(parentFile);
        providedHeaders.push(parentFile);
      }
    }
  }
  
  //--- Remove unnecessary headers ---//
  
  for (const model::FileId& header : usedHeaders)
    if (keptHeaders.find(header) == keptHeaders.end())
      decorator.delNode(header);

  //--- Container binaries ---//

  for (const model::FileId& source : providerSources)
    util::diagram::bfs(
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
