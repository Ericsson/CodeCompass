#ifndef CC_SERVICE_PYTHON_DIAGRAM_H
#define CC_SERVICE_PYTHON_DIAGRAM_H

#include <model/pyname.h>
#include <util/graph.h>
#include <service/pythonservice.h>
#include <string>
#include <vector>

namespace cc
{
namespace service
{
namespace language
{
namespace python
{    
  class Diagram {
    public:
      Diagram(
        std::shared_ptr<odb::database> db_,
        std::shared_ptr<std::string> datadir_,
        const cc::webserver::ServerContext& context_);

      util::Graph getFunctionCallDiagram(const model::PYName& pyname);

      enum NodeDecoration {
        FunctionCenterNode,
        FunctionCallNode,
        FunctionCallDefinitionNode,
        FunctionCallerNode,
        FunctionCallerDefinitionNode
      };
    private:
      PythonServiceHandler m_pythonService;
      std::vector<model::PYName> functionGoto(const std::vector<model::PYName>& functions, const PythonServiceHandler::ReferenceType& ref_type);
      void decorateNode(util::Graph& graph_, util::Graph::Node& node_, const NodeDecoration& decoration);
      util::Graph::Node addNode(util::Graph& graph_, const model::PYName& pyname);
  };
} // python
} // language
} // service
} // cc

#endif
