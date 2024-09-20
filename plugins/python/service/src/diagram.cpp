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
  // Query calls
  const std::vector<model::PYName> this_calls = m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::THIS_CALLS);
  std::vector<std::uint64_t> this_calls_refs = m_pythonService.transformReferences(this_calls, model::REF_ID);
  const std::vector<model::PYName> calls = m_pythonService.queryNodes(odb::query<model::PYName>::ref_id.in_range(this_calls_refs.begin(), this_calls_refs.end()) && odb::query<model::PYName>::is_definition == true && odb::query<model::PYName>::is_import == false);

  // Query callers
  const std::vector<model::PYName> function_callers = m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::CALLER);
  std::vector<std::uint64_t> callers_parents = m_pythonService.transformReferences(function_callers, model::PARENT_FUNCTION);
  const std::vector<model::PYName> callers = m_pythonService.queryNodes(odb::query<model::PYName>::id.in_range(callers_parents.begin(), callers_parents.end()));

  // Create graph
  util::Graph graph;
  graph.setAttribute("rankdir", "LR");

  // Add center node
  util::Graph::Node centerNode = addPYNameNode(graph, pyname, true);
  const std::string centerNodeID = std::to_string(pyname.id);
  decorateNode(graph, centerNode, FunctionCenterNode);

  // Add calls with definitions
  std::unordered_map<std::uint64_t, bool> addedNodes;
  for (const model::PYName& node : calls)
  {
    addFunctionNode(graph, centerNode, node, FunctionCallDefinitionNode);
    addedNodes.emplace(node.ref_id, true);
  }

  // Add calls with missing definitions
  for(const model::PYName& node : this_calls)
  {
    if(addedNodes.find(node.ref_id) == addedNodes.end())
    {
      addFunctionNode(graph, centerNode, node, FunctionCallNode);
    }
  }

  addedNodes.clear();

  // Add callers with definitions
  for (const model::PYName& node : callers)
  {
    addFunctionNode(graph, centerNode, node, FunctionCallerDefinitionNode);
  }

  // Add callers with missing definitions
  for (const model::PYName& node : function_callers)
  {
    if(node.parent_function == node.id)
    {
      addFunctionNode(graph, centerNode, node, FunctionCallerNode);
    }
  }

  return graph;
}

util::Graph Diagram::getModuleDiagram(const core::FileId& fileId)
{
    util::Graph graph;
    graph.setAttribute("rankdir", "LR");

    const std::uint64_t file_id = std::stoull(fileId);
    core::FileInfo mainFileInfo;
    m_projectService.getFileInfo(mainFileInfo, fileId);
    util::Graph::Node centerNode = addFileNode(graph, mainFileInfo);
    decorateNode(graph, centerNode, FilePathCenterNode);

    // Query nodes
    const std::vector<std::uint64_t> nodesInFile = m_pythonService.transformReferences(m_pythonService.queryNodesInFile(fileId, false), model::REF_ID);
    const std::vector<std::uint64_t> definitionsInFile = m_pythonService.transformReferences(m_pythonService.queryNodesInFile(fileId, true), model::REF_ID);

    const std::vector<model::PYName> importedDefinitions = m_pythonService.queryNodes(
      odb::query<model::PYName>::ref_id.in_range(nodesInFile.begin(), nodesInFile.end()) && odb::query<model::PYName>::is_definition == true &&
      odb::query<model::PYName>::is_import == false && odb::query<model::PYName>::file_id != file_id &&
      !(odb::query<model::PYName>::line_start == 0 && odb::query<model::PYName>::line_end == 0 && odb::query<model::PYName>::column_start == 1 && odb::query<model::PYName>::column_end == 1));

    const std::vector<model::PYName> importedUsages = m_pythonService.queryNodes(
      odb::query<model::PYName>::ref_id.in_range(definitionsInFile.begin(), definitionsInFile.end()) && odb::query<model::PYName>::is_definition == false &&
      odb::query<model::PYName>::file_id != file_id);

    std::unordered_map<model::FileId, util::Graph::Node> map;

    auto getFileNode = [&](const model::PYName& p, const NodeType& nodeType)
    {
      core::FileInfo fileInfo;
      try {
        m_projectService.getFileInfo(fileInfo, std::to_string(p.file_id));
      } catch (core::InvalidId) {
        return util::Graph::Node();
      }

      auto it = map.find(p.file_id);

      if (it == map.end())
      {
        util::Graph::Node n = addFileNode(graph, fileInfo, centerNode, nodeType);

        map.emplace(p.file_id, n);
        return n;
      }else{
        return it->second;
      }
    };

    for (const model::PYName& p : importedDefinitions)
    {
      util::Graph::Node node = getFileNode(p, ImportedFilePathNode);
      if (node.empty()) continue;

      util::Graph::Node graphNode = addPYNameNode(graph, p, false);
      decorateNode(graph, graphNode, ImportedNode);
      graph.createEdge(node, graphNode);
    }

    map.clear();

    std::unordered_map<std::uint64_t, bool> sameLine;
    for (const model::PYName& p : importedUsages)
    {
      util::Graph::Node node = getFileNode(p, ImportsFilePathNode);
      if (node.empty()) continue;

      if (p.line_start == p.line_end && sameLine.find(p.line_start) != sameLine.end()) continue;

      util::Graph::Node graphNode = addPYNameNode(graph, p, false);
      decorateNode(graph, graphNode, ImportsNode);
      graph.createEdge(graphNode, node);

      sameLine.emplace(p.line_start, true);
    }
    return graph;
}

void Diagram::addFunctionNode(util::Graph& graph_, const util::Graph::Node& centerNode, const model::PYName& pyname, const NodeType& nodeType)
{
    util::Graph::Node node = addPYNameNode(graph_, pyname, true);
    decorateNode(graph_, node, nodeType);

    if(nodeType == FunctionCallNode || nodeType == FunctionCallDefinitionNode)
    {
      graph_.createEdge(centerNode, node);
    }
    else
    {
      graph_.createEdge(node, centerNode);
    }
}

util::Graph::Node Diagram::addPYNameNode(util::Graph& graph_, const model::PYName& pyname, bool addSubgraph)
{
  const util::Graph::Subgraph subgraph = [&]()
  {
    if(!addSubgraph) return util::Graph::Subgraph();

    const core::FileId fileId = std::to_string(pyname.file_id);
    auto it = m_subgraphs.find(fileId);

    if (it != m_subgraphs.end())
      return it->second;

    core::FileInfo fileInfo;
    try {
      m_projectService.getFileInfo(fileInfo, fileId);
    } catch (core::InvalidId) {
      return util::Graph::Subgraph();
    }

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

util::Graph::Node Diagram::addFileNode(util::Graph& graph_, const core::FileInfo& fileInfo)
{
  util::Graph::Node node = graph_.getOrCreateNode("f" + fileInfo.id);
  graph_.setNodeAttribute(node, "label", fileInfo.path);

  return node;
}

util::Graph::Node Diagram::addFileNode(util::Graph& graph_, const core::FileInfo& fileInfo, const util::Graph::Node& centerNode, const NodeType& nodeType)
{
  const std::string id = (nodeType == ImportedFilePathNode) ? "d" + fileInfo.id : "s" + fileInfo.id;

  util::Graph::Node node = graph_.getOrCreateNode(id);
  graph_.setNodeAttribute(node, "label", fileInfo.path);
  decorateNode(graph_, node, nodeType);

  if (nodeType == ImportedFilePathNode) {
    graph_.createEdge(centerNode, node);
  } else {
    graph_.createEdge(node, centerNode);
  }

  return node;
}

void Diagram::decorateNode(util::Graph& graph_, util::Graph::Node& node_, const NodeType& nodeType)
{
  graph_.setNodeAttribute(node_, "style", "filled");

  switch(nodeType)
  {
    case FunctionCenterNode:
      graph_.setNodeAttribute(node_, "fillcolor", "gold");
      break;
    case FunctionCallerNode:
      graph_.setNodeAttribute(node_, "fillcolor", "orange");
      graph_.setNodeAttribute(node_, "shape", "cds");
      break;
    case FunctionCallerDefinitionNode:
      graph_.setNodeAttribute(node_, "fillcolor", "coral");
      break;
    case FunctionCallNode:
      graph_.setNodeAttribute(node_, "fillcolor", "cyan");
      graph_.setNodeAttribute(node_, "shape", "cds");
      break;
    case FunctionCallDefinitionNode:
      graph_.setNodeAttribute(node_, "fillcolor", "lightblue");
      break;
    case FilePathCenterNode:
      graph_.setNodeAttribute(node_, "fillcolor", "gold");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case ImportedFilePathNode:
      graph_.setNodeAttribute(node_, "fillcolor", "limegreen");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case ImportedNode:
      graph_.setNodeAttribute(node_, "fillcolor", "lightseagreen");
      break;
    case ImportsFilePathNode:
      graph_.setNodeAttribute(node_, "fillcolor", "orange");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case ImportsNode:
      graph_.setNodeAttribute(node_, "fillcolor", "coral");
      graph_.setNodeAttribute(node_, "shape", "cds");
      break;
  }
}
} // python
} // language
} // service
} // cc
