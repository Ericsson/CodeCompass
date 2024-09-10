#include "diagram.h"

namespace
{
  using namespace cc;

  util::Graph::Node addNode(util::Graph& graph_, const model::PYName& pyname)
  {
    util::Graph::Node node = graph_.getOrCreateNode(std::to_string(pyname.id));
    graph_.setNodeAttribute(node, "label", pyname.value);

    return node;
  }
}

namespace cc
{
namespace service
{
namespace language
{  
namespace python
{    
  void getFunctionCallDiagram(util::Graph& graph_, const model::PYName& pyname, const std::vector<model::PYName>& calls)
  {
    util::Graph::Node centerNode = addNode(graph_, pyname);

    for (const model::PYName& node : calls)
    {
      util::Graph::Node callNode = addNode(graph_, node);
      util::Graph::Edge edge = graph_.createEdge(centerNode, callNode);
    }
  }
} // python
} // language
} // service
} // cc
