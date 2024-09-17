#include "diagram.h"

namespace cc
{
namespace service
{
namespace language
{  
namespace python
{    
Diagram::Diagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : m_pythonService(db_, datadir_, context_),
      m_projectService(db_, datadir_, context_){}

util::Graph Diagram::getFunctionCallDiagram(const model::PYName& pyname)
{
  const std::vector<model::PYName> callers = functionGoto(m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::CALLER), PythonServiceHandler::PARENT_FUNCTION);
  const std::vector<model::PYName> calls = functionGoto(m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::THIS_CALLS), PythonServiceHandler::DEFINITION);

  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  util::Graph::Node centerNode = addNode(graph, pyname);
  const std::string centerNodeID = std::to_string(pyname.id);
  decorateNode(graph, centerNode, FunctionCenterNode);

  for (const model::PYName& node : calls)
  {
    util::Graph::Node callNode = addNode(graph, node);
    decorateNode(graph, callNode, node.is_definition ? FunctionCallDefinitionNode : FunctionCallNode);

    if(!node.is_definition)
    {
      graph.setNodeAttribute(callNode, "id", centerNodeID);
    }

    util::Graph::Edge edge = graph.createEdge(centerNode, callNode);
  }

  for (const model::PYName& node : callers)
  {
    util::Graph::Node callerNode = addNode(graph, node);
    decorateNode(graph, callerNode, node.is_definition ? FunctionCallerDefinitionNode : FunctionCallerNode);

    if(!node.is_definition)
    {
      graph.setNodeAttribute(callerNode, "id", centerNodeID);
    }

    util::Graph::Edge edge = graph.createEdge(callerNode, centerNode);
  }

  return graph;
}

std::vector<model::PYName> Diagram::functionGoto(const std::vector<model::PYName>& functions, const PythonServiceHandler::ReferenceType& ref_type)
{
  std::vector<model::PYName> calls;
  std::unordered_map<std::uint64_t, bool> added;

  // Find reference for each function
  // Example use case: Find definition for each function
  for(const model::PYName& p : functions)
  {
    std::vector<model::PYName> defs = m_pythonService.queryReferences(std::to_string(p.id), ref_type);
    if(defs.size() == 1)
    {
      model::PYName d = defs[0];

      if(d.value.empty())
      {
        calls.push_back(p);
        continue;
      }

      if(added.count(d.id) == 0)
      {
        calls.push_back(d);
        added.emplace(d.id, true);
      }
    }else{
      calls.push_back(p);
    }
  }

  return calls;
}

util::Graph::Node Diagram::addNode(util::Graph& graph_, const model::PYName& pyname)
{
  const util::Graph::Subgraph subgraph = [&]()
  {
    const core::FileId fileId = std::to_string(pyname.file_id);
    auto it = m_subgraphs.find(fileId);

    if (it != m_subgraphs.end())
      return it->second;

    core::FileInfo fileInfo;
    m_projectService.getFileInfo(fileInfo, fileId);

    util::Graph::Subgraph subgraph
      = graph_.getOrCreateSubgraph("cluster_" + fileInfo.path);

    graph_.setSubgraphAttribute(subgraph, "id", fileInfo.id);
    const std::string coloredLabel =
      "<table border=\"0\" cellborder=\"0\"><tr><td bgcolor=\"limegreen\">" +
      fileInfo.path +
      "</td></tr></table>";
    graph_.setSubgraphAttribute(subgraph, "label", coloredLabel, true);

    m_subgraphs.insert(it, std::make_pair(fileInfo.path, subgraph));

    return subgraph;
  }();

  util::Graph::Node node = graph_.getOrCreateNode(std::to_string(pyname.id), subgraph);

  std::string label = pyname.value;

  if(!pyname.is_definition)
  {
    label = m_pythonService.getNodeLineValue(pyname);
  }

  graph_.setNodeAttribute(node, "label", label);

  return node;
}

void Diagram::decorateNode(util::Graph& graph_, util::Graph::Node& node_, const NodeDecoration& decoration)
{
  graph_.setNodeAttribute(node_, "style", "filled");

  switch(decoration)
  {
    case FunctionCenterNode:
      graph_.setNodeAttribute(node_, "fillcolor", "gold");
      break;
    case FunctionCallerNode:
      graph_.setNodeAttribute(node_, "fillcolor", "orange");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case FunctionCallerDefinitionNode:
      graph_.setNodeAttribute(node_, "fillcolor", "coral");
      break;
    case FunctionCallNode:
      graph_.setNodeAttribute(node_, "fillcolor", "cyan");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case FunctionCallDefinitionNode:
      graph_.setNodeAttribute(node_, "fillcolor", "lightblue");
      break;
  }
}
} // python
} // language
} // service
} // cc
