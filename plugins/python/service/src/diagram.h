#ifndef CC_SERVICE_PYTHON_DIAGRAM_H
#define CC_SERVICE_PYTHON_DIAGRAM_H

#include <model/pyname.h>
#include <util/graph.h>
#include <service/pythonservice.h>
#include <string>

namespace cc
{
namespace service
{
namespace language
{
namespace python
{    
  void getFunctionCallDiagram(util::Graph& graph_, const model::PYName& pyname, const std::vector<model::PYName>& calls);
} // python
} // language
} // service
} // cc

#endif
