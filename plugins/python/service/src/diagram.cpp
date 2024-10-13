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
  decorateNode(graph, centerNode, CenterNode);

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
      } catch (const core::InvalidId&) {
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
      util::Graph::Node node = getFileNode(p, p.is_builtin ? ImportedBuiltinFilePathNode : ImportedFilePathNode);
      if (node.empty()) continue;

      util::Graph::Node graphNode = addPYNameNode(graph, p, false);
      decorateNode(graph, graphNode, ImportedNode);
      graph.createEdge(node, graphNode);
    }

    map.clear();

    for (const model::PYName& p : importedUsages)
    {
      util::Graph::Node node = getFileNode(p, ImportsFilePathNode);
    }
    return graph;
}

util::Graph Diagram::getUsageDiagram(const model::PYName& pyname)
{
    util::Graph graph;
    graph.setAttribute("rankdir", "LR");
    util::Graph::Node centerNode = addPYNameNode(graph, pyname, true);
    decorateNode(graph, centerNode, CenterNode);

    if (!pyname.is_definition || pyname.is_import == true) return graph;

    const std::vector<model::PYName> usages = m_pythonService.queryNodes(
      odb::query<model::PYName>::ref_id == pyname.ref_id && odb::query<model::PYName>::is_definition == false);

    for (const model::PYName& p : usages)
    {
      util::Graph::Node graphNode = addPYNameNode(graph, p, true);
      decorateNode(graph, graphNode, p.is_call ? FunctionCallNode : UsageNode);
      graph.createEdge(graphNode, centerNode);
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

util::Graph::Subgraph Diagram::getFileSubgraph(util::Graph& graph_, const model::PYName& pyname)
{
    const core::FileId fileId = std::to_string(pyname.file_id);
    auto it = m_subgraphs.find(fileId);

    if (it != m_subgraphs.end()) {
      return it->second;
    }

    core::FileInfo fileInfo;
    try {
      m_projectService.getFileInfo(fileInfo, fileId);
    } catch (const core::InvalidId&) {
      return util::Graph::Subgraph();
    }

    util::Graph::Subgraph subgraph = graph_.getOrCreateSubgraph("cluster_" + fileInfo.path);
    graph_.setSubgraphAttribute(subgraph, "id", fileInfo.id);

    const std::string pathColor = (pyname.is_builtin && pyname.is_definition) ? "dodgerblue" : "limegreen";
    const std::string coloredLabel =
      "<table border=\"0\" cellborder=\"0\"><tr><td bgcolor=\"" + pathColor + "\">" +
      fileInfo.path +
      "</td></tr></table>";
    graph_.setSubgraphAttribute(subgraph, "label", coloredLabel, true);

    m_subgraphs.emplace(fileInfo.path, subgraph);

    return subgraph;
}

util::Graph::Node Diagram::addPYNameNode(util::Graph& graph_, const model::PYName& pyname, bool addSubgraph)
{
  const util::Graph::Subgraph subgraph = (addSubgraph) ? getFileSubgraph(graph_, pyname) : util::Graph::Subgraph();

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
  /* We might need to add a file path multiple times to the diagram.
    Since we need unique ids, we will differentiate nodes based on starting character:
    'f' - regular file path node
    'd' - ImportedFilePathNode, ImportedBuiltinFilePathNode
    's' - ImportsFilePathNode
    Any id without a {'f', 'd', 's'} starting character is not a file path node. */
  const std::string id = (nodeType == ImportedFilePathNode || nodeType == ImportedBuiltinFilePathNode) ? "d" + fileInfo.id : "s" + fileInfo.id;

  util::Graph::Node node = graph_.getOrCreateNode(id);
  graph_.setNodeAttribute(node, "label", fileInfo.path);
  decorateNode(graph_, node, nodeType);

  if (nodeType == ImportedFilePathNode || nodeType == ImportedBuiltinFilePathNode) {
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
    case CenterNode:
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
      graph_.setNodeAttribute(node_, "fillcolor", "deepskyblue");
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
    case ImportedBuiltinFilePathNode:
      graph_.setNodeAttribute(node_, "fillcolor", "dodgerblue");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case ImportedNode:
      graph_.setNodeAttribute(node_, "fillcolor", "lightseagreen");
      break;
    case ImportsFilePathNode:
      graph_.setNodeAttribute(node_, "fillcolor", "orange");
      graph_.setNodeAttribute(node_, "shape", "box");
      break;
    case UsageNode:
      graph_.setNodeAttribute(node_, "fillcolor", "cyan");
      graph_.setNodeAttribute(node_, "shape", "cds");
      break;
  }
}

util::Graph Diagram::getFunctionCallDiagramLegend()
{
    util::Graph graph;
    graph.setAttribute("rankdir", "LR");

    addLegendNode(graph, ImportedBuiltinFilePathNode, "Builtin file path", false);
    addLegendNode(graph, ImportedFilePathNode, "File path", false);
    addLegendNode(graph, FunctionCallNode, "Function call without definition");
    addLegendNode(graph, FunctionCallDefinitionNode, "Function call");
    addLegendNode(graph, FunctionCallerNode, "Line of code calling selected node");
    addLegendNode(graph, FunctionCallerDefinitionNode, "Function calling selected node");
    addLegendNode(graph, CenterNode, "Selected node");

    return graph;
}

util::Graph Diagram::getUsageDiagramLegend()
{
    util::Graph graph;
    graph.setAttribute("rankdir", "LR");

    addLegendNode(graph, ImportedBuiltinFilePathNode, "Builtin module path", false);
    addLegendNode(graph, ImportedFilePathNode, "Module path", false);
    addLegendNode(graph, FunctionCallNode, "Function call");
    addLegendNode(graph, UsageNode, "Usage node");
    addLegendNode(graph, CenterNode, "Selected node");

    return graph;
}

util::Graph Diagram::getModuleDiagramLegend()
{
    util::Graph graph;
    graph.setAttribute("rankdir", "LR");

    addLegendNode(graph, ImportedNode, "Imported definition");
    addLegendNode(graph, ImportedBuiltinFilePathNode, "Imported builtin module");
    addLegendNode(graph, ImportedFilePathNode, "Imported module");
    addLegendNode(graph, ImportsFilePathNode, "Module importing selected node");
    addLegendNode(graph, FilePathCenterNode, "Selected node");

    return graph;
}

void Diagram::addLegendNode(util::Graph& graph_, const NodeType& nodeType, const std::string& text, bool shape)
{
  util::Graph::Node node = graph_.createNode();
  graph_.setNodeAttribute(node, "label", "");
  decorateNode(graph_, node, nodeType);

  if (!shape) {
    graph_.setNodeAttribute(node, "shape", "plaintext");
  }

  const util::Graph::Node explanation = graph_.createNode();
  graph_.setNodeAttribute(explanation, "shape", "none");

  const util::Graph::Edge edge = graph_.createEdge(node, explanation);
  graph_.setEdgeAttribute(edge, "style", "invis");

  graph_.setNodeAttribute(explanation, "label", text);
}

util::Graph Diagram::getClassDiagram(const model::PYName& pyname)
{
    util::Graph graph;
    graph.setAttribute("rankdir", "BT");

    auto setAttributes = [this, &graph](const util::Graph::Node& node, const model::PYName& p) {
      graph.setNodeAttribute(node, "fontname", "Noto Serif");
      graph.setNodeAttribute(node, "shape", "plaintext");
      graph.setNodeAttribute(node, "label", getClassTable(p), true);
    };

    util::Graph::Node classNode = graph.getOrCreateNode(std::to_string(pyname.id), getFileSubgraph(graph, pyname));
    setAttributes(classNode, pyname);

    // Query baseclasses
    const std::vector<model::PYName> bases = m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::BASE_CLASS);

    for (const model::PYName& p : bases) {
      if (p.type != "class") {
        continue;
      }

      util::Graph::Node node = graph.getOrCreateNode(std::to_string(p.id), getFileSubgraph(graph, p));
      setAttributes(node, p);

      util::Graph::Edge edge = graph.createEdge(classNode, node);
      graph.setEdgeAttribute(edge, "arrowhead", "empty");
    }

    return graph;
}

std::string Diagram::getClassTable(const model::PYName& pyname)
{
    auto getVisibility = [](const std::string& str)
    {
      if (str.substr(0, 6) == "def __") {
        return std::string("<font color=\"red\"><b>- </b></font>");
      } else {
        return std::string("<font color=\"green\"><b>+ </b></font>");
      }
    };

    auto highlightVariable = [](const std::string& str, const std::string& baseColor)
    {
      // Remove comma
      std::string p = (str.back() == ',') ? str.substr(0, str.size() - 1) : str;

      const size_t col = p.find(":");
      const size_t eq = p.find("=");

      if (col == std::string::npos) {
        p = "<font color=\"" + baseColor + "\">" + p + "</font>";
      } else if (eq == std::string::npos) {
        p = "<font color=\"" + baseColor + "\">" + p.substr(0, col) + "</font>" + ":" +
          "<font color=\"darkorange\">" + p.substr(col + 1) + "</font>";
      } else {
        p = "<font color=\"" + baseColor + "\">" + p.substr(0, col) + "</font>" + ":" +
          "<font color=\"darkorange\">" + p.substr(col + 1, eq - col - 1) + "</font>" +
          "<font color=\"" + baseColor + "\">" + p.substr(eq) + "</font>";
      }

      return p;
    };

    auto getSignature = [this, &highlightVariable](const model::PYName& p)
    {
      const size_t opening = p.value.find('(');
      if (p.value.substr(0, 3) != "def" || opening == std::string::npos) {
        return highlightVariable(p.value, "black");
      }

      // Remove "def"
      std::string sign = p.value.substr(3, opening - 3);
      sign += '(';

      // Query params
      const std::vector<model::PYName> params = m_pythonService.queryReferences(std::to_string(p.id), PythonServiceHandler::PARAMETER);

      // Add params to signature
      bool first = true;
      for (const model::PYName& e : params) {
        // Skip param "self"
        if (first && e.value.substr(0,4) == "self") {
          continue;
        }

        if (first) {
          first = false;
        } else {
          sign += ", ";
        }

        sign += highlightVariable(e.value, "darkgreen");
      }

      sign += ')';

      // Query return annotation
      const std::vector<model::PYName> annotations = m_pythonService.queryReferences(std::to_string(p.id), PythonServiceHandler::ANNOTATION);
      if(annotations.size() == 1) {
        sign += "<font color=\"blue\">  ->  " + annotations[0].value + "</font>";
      }

      return sign;
    };

    // Query nodes
    const std::vector<model::PYName> data_members = m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::DATA_MEMBER);
    const std::vector<model::PYName> methods = m_pythonService.queryReferences(std::to_string(pyname.id), PythonServiceHandler::METHOD);

    std::string label = "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\"><tr><td bgcolor=\"dodgerblue\">" + pyname.value + "</td></tr>";

    label += "<tr><td align=\"left\">";
    for (const model::PYName& p : data_members) {
      label += "<br align=\"left\" />" + getVisibility(p.value) + getSignature(p) + "<br align=\"left\" />";
    }
    label += "</td></tr>";

    label += "<tr><td align=\"left\">";
    for (const model::PYName& p : methods) {
      label += "<br align=\"left\" />" + getVisibility(p.value) + getSignature(p) + "<br align=\"left\" />";
    }
    label += "</td></tr>";
    label += "</table>";
    return label;
}
} // python
} // language
} // service
} // cc
