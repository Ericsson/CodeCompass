#include <util/util.h>

#include "pointeranalysisstmtcollector.h"
#include "andersen.h"
#include "steensgaard.h"
#include "pointeranalysisdiagram.h"

namespace
{

typedef odb::query<cc::model::CppAstNode> AstQuery;
typedef odb::result<cc::model::CppAstNode> AstResult;

}

namespace cc
{
namespace service
{
namespace language
{

PointerAnalysisDiagram::PointerAnalysisDiagram(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const boost::program_options::variables_map& config_)
    : _db(db_),
      _transaction(db_),
      _cppHandler(db_, datadir_, config_),
      _projectHandler(db_, datadir_, config_)
{
}

void PointerAnalysisDiagram::getAndersenPointerAnalysisDiagram(
  util::Graph& graph_,
  const core::AstNodeId& astNodeId_)
{
  graph_.setAttribute("rankdir", "LR");

  std::vector<AstNodeInfo> nodes;
  _cppHandler.getReferences(
    nodes, astNodeId_, CppServiceHandler::DEFINITION, {});

  if (nodes.empty())
    return;

  AstNodeInfo node = nodes.front();

  //--- Create subraphs for memory models: global, stack, heap ---//

  createMemorySubraphs(graph_);

  //--- Collect statements ---//

  PointerAnalysisStmtCollector collector(_db);
  std::vector<model::CppPointerAnalysis> statements =
    collector.collect(node.mangledNameHash);

  //--- Run Andersen style points-to algorithm on the collected statements ---//

  Andersen algorithm;
  Andersen::PointsToSet pointsToSet = algorithm.run(statements);

  //--- Draw pointer analysis diagram ---//

  std::unordered_map<std::uint64_t, model::CppAstNode> astNodeCache;
  _transaction([&, this]{
    for (const auto& key : pointsToSet)
    {
      model::CppPointerAnalysis::StmtSide lhsSide = key.first;

      if (!getAstNode(lhsSide.mangledNameHash, astNodeCache))
      {
        LOG(warning)
          << "No CppAstNode for lhs side: " << lhsSide.mangledNameHash;
        continue;
      }

      model::CppAstNode& lhsNode = astNodeCache.at(lhsSide.mangledNameHash);

      std::string lhsNodeId = std::to_string(lhsNode.id);
      if (!graph_.hasNode(lhsNodeId))
      {
        createPointerAnalysisNode(graph_, lhsNodeId, key.first.options, true);
        decoratePointerAnalysisNode(graph_, lhsNode, key.first.options);
      }

      for (const auto& value : key.second)
      {
        const model::CppPointerAnalysis::StmtSide& rhsSide = value;

        if (!getAstNode(value.mangledNameHash, astNodeCache))
        {
          LOG(warning)
            << "No CppAstNode for rhs side: " << value.mangledNameHash;
          continue;
        }

        model::CppAstNode& rhsNode = astNodeCache.at(value.mangledNameHash);

        std::string rhsNodeId = std::to_string(rhsNode.id);

        if (!graph_.hasNode(rhsNodeId))
        {
          createPointerAnalysisNode(graph_, rhsNodeId, value.options, true);
          decoratePointerAnalysisNode(graph_, rhsNode, value.options);
        }

        util::Graph::Edge edge = graph_.createEdge(lhsNodeId, rhsNodeId);
        graph_.setEdgeAttribute(edge, "id", lhsNodeId + "_" + rhsNodeId);
        decoratePointerAnalysisEdge(
          graph_, edge, lhsSide.options, rhsSide.options);
      }
    }
  });

  if (graph_.hasNode(node.id))
    decorateNode(graph_, node.id, centerNodeDecoration);
}

void PointerAnalysisDiagram::getSteensgaardPointerAnalysisDiagram(
  util::Graph& graph_,
  const core::AstNodeId& astNodeId_)
{
  graph_.setAttribute("rankdir", "LR");

  std::vector<AstNodeInfo> nodes;
  _cppHandler.getReferences(
    nodes, astNodeId_, CppServiceHandler::DEFINITION, {});

  if (nodes.empty())
    return;

  AstNodeInfo node = nodes.front();

  //--- Create subraphs for memory models: global, stack, heap ---//

  createMemorySubraphs(graph_);

  //--- Collect statements ---//

  PointerAnalysisStmtCollector collector(_db);
  std::vector<model::CppPointerAnalysis> statements =
    collector.collect(node.mangledNameHash);

  //--- Run Steensgaard style points-to algorithm on the collected statements ---//

  Steensgaard algorithm;
  std::map<model::CppPointerAnalysis::StmtSide, Steensgaard::TypeNodePtr> res =
    algorithm.run(statements);

  //--- Draw pointer analysis diagram ---//

  std::unordered_map<std::uint64_t, model::CppAstNode> astNodeCache;
  _transaction([&, this]{
    for (const auto& key : res)
    {
      model::CppPointerAnalysis::StmtSide lhsSide = key.first;

      if (!getAstNode(lhsSide.mangledNameHash, astNodeCache))
      {
        LOG(warning)
          << "No CppAstNode for lhs side: " << lhsSide.mangledNameHash;
        continue;
      }

      model::CppAstNode& lhsNode = astNodeCache.at(lhsSide.mangledNameHash);
      std::string lhsNodeId = std::to_string(lhsNode.id);

      if (!graph_.hasNode(lhsNodeId))
      {
        std::string subgraphId =
          std::to_string(key.second->value.mangledNameHash);
        util::Graph::Subgraph subgraph =
          graph_.getOrCreateSubgraph(subgraphId);
        graph_.getOrCreateNode(lhsNodeId, subgraph);

        decoratePointerAnalysisNode(graph_, lhsNode, key.first.options);
      }

      if (!key.second->pointsTo)
         continue;

      model::CppPointerAnalysis::StmtSide rhsSide = key.second->pointsTo->value;

      if (!getAstNode(rhsSide.mangledNameHash, astNodeCache))
      {
        LOG(warning)
          << "No CppAstNode for rhs side: " << rhsSide.mangledNameHash;
        continue;
      }

      model::CppAstNode& rhsNode = astNodeCache.at(rhsSide.mangledNameHash);
      std::string rhsNodeId = std::to_string(rhsNode.id);

      if (!graph_.hasNode(rhsNodeId))
      {
        std::string subgraphId = std::to_string(rhsSide.mangledNameHash);
        util::Graph::Subgraph subgraph =
          graph_.getOrCreateSubgraph(subgraphId);
        graph_.getOrCreateNode(rhsNodeId, subgraph);

        decoratePointerAnalysisNode(graph_, rhsNode, rhsSide.options);
      }

      util::Graph::Edge edge = graph_.createEdge(lhsNodeId, rhsNodeId);
      graph_.setEdgeAttribute(edge, "id", lhsNodeId + "_" + rhsNodeId);
      decoratePointerAnalysisEdge(
        graph_, edge, lhsSide.options, rhsSide.options);
    }
  });

  if (graph_.hasNode(node.id))
    decorateNode(graph_, node.id, centerNodeDecoration);
}

bool PointerAnalysisDiagram::getAstNode(
  std::uint64_t mangledNameHash_,
  std::unordered_map<std::uint64_t, model::CppAstNode>& astNodeCache_)
{
  if (!astNodeCache_.count(mangledNameHash_))
  {
    AstResult def = _db->query<model::CppAstNode>(
      AstQuery::mangledNameHash == mangledNameHash_ &&
      ( AstQuery::astType == model::CppAstNode::AstType::Declaration ||
        AstQuery::astType == model::CppAstNode::AstType::Definition  ||
        AstQuery::astType == model::CppAstNode::AstType::Other));

    if (def.empty())
      return false;

    astNodeCache_.insert(std::make_pair(mangledNameHash_, *def.begin()));
  }

  return true;
}

void PointerAnalysisDiagram::createPointerAnalysisNode(
  util::Graph& graph_,
  const std::string& nodeId_,
  model::CppPointerAnalysis::Options_t options_,
  bool addToSubraphs)
{
  if (!graph_.hasNode(nodeId_))
  {
    std::string cluster;

    if (addToSubraphs)
    {
      if (options_ & model::CppPointerAnalysis::Options::HeapObj)
        cluster = "cluster_heap";
      else if (options_ & model::CppPointerAnalysis::Options::StackObj)
        cluster = "cluster_stack";
      else if (options_ & model::CppPointerAnalysis::Options::GlobalObject)
        cluster = "cluster_global";

      graph_.getOrCreateNode(nodeId_, cluster);
    }

    if (cluster.empty())
      graph_.getOrCreateNode(nodeId_);
  }
}

void PointerAnalysisDiagram::decoratePointerAnalysisEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  model::CppPointerAnalysis::Options_t lhsOptions_,
  model::CppPointerAnalysis::Options_t rhsOptions_)
{
  if (lhsOptions_ & model::CppPointerAnalysis::Options::Reference)
    decorateEdge(graph_, edge_, referenceEdgeDecoration);
  else if (rhsOptions_ & model::CppPointerAnalysis::Options::InitList)
    decorateEdge(graph_, edge_, initListEdgeDecoration);
}

void PointerAnalysisDiagram::decoratePointerAnalysisNode(
  util::Graph& graph_,
  const model::CppAstNode& astNode,
  model::CppPointerAnalysis::Options_t options_)
{
  std::string nodeId = std::to_string(astNode.id);

  std::string label = astNode.astValue.empty()
    ? "<unnamed variable>"
    : astNode.astValue;

  graph_.setNodeAttribute(nodeId, "label", label);

  if (options_ & model::CppPointerAnalysis::Options::FunctionCall)
    decorateNode(graph_, nodeId, functionNodeDecoration);
  if (options_ & model::CppPointerAnalysis::Options::NullPtr)
    decorateNode(graph_, nodeId , nullptrNodeDecoration);
  if (options_ & model::CppPointerAnalysis::Options::Undefined)
    decorateNode(graph_, nodeId , undefinedNodeDecoration);
  if (options_ & model::CppPointerAnalysis::Options::InitList)
    decorateNode(graph_, nodeId , initListNodeDecoration);
  if (options_ & model::CppPointerAnalysis::Options::Array)
  {
    graph_.setNodeAttribute(
      nodeId, "shape", "record");
    graph_.setNodeAttribute(
      nodeId, "label", "{" + astNode.astValue + "} | {...|...}");
  }
  if (options_ & (model::CppPointerAnalysis::Options::HeapObj |
                  model::CppPointerAnalysis::Options::Literal))
    decorateNode(graph_, nodeId , objectNodeDecoration);
}

void PointerAnalysisDiagram::createMemorySubraphs(util::Graph& graph_)
{
  util::Graph::Subgraph heapObjContainer =
    graph_.getOrCreateSubgraph("heap");
  decorateSubgraph(graph_, heapObjContainer, heapMemorySubgraphDecoration);

  util::Graph::Subgraph stackObjContainer =
    graph_.getOrCreateSubgraph("stack");
  decorateSubgraph(graph_, stackObjContainer, stackMemorySubgraphDecoration);

  util::Graph::Subgraph globalObjContainer =
    graph_.getOrCreateSubgraph("global");
  decorateSubgraph(graph_, globalObjContainer, globalMemorySubgraphDecoration);
}

std::string PointerAnalysisDiagram::getAndersenDiagramLegend()
{
  util::LegendBuilder builder("Andersen's Points-to Diagram");

  getPointerAnalysisDiagramLegend(builder);

  return builder.getOutput();
}

std::string PointerAnalysisDiagram::getSteensgaardDiagramLegend()
{
  util::LegendBuilder builder("Steensgaard's Points-to Diagram");

  getPointerAnalysisDiagramLegend(builder);

  return builder.getOutput();
}

void PointerAnalysisDiagram::getPointerAnalysisDiagramLegend(
  util::LegendBuilder& builder_)
{
  builder_.addNode("center node", centerNodeDecoration);
  builder_.addNode("variable on the stack", stackNodeDecoration);
  builder_.addNode("heap object or string literal", objectNodeDecoration);
  builder_.addNode("undefined memory", undefinedNodeDecoration);
  builder_.addNode("NULL/nullptr expression", nullptrNodeDecoration);
  builder_.addNode("function", functionNodeDecoration);
  builder_.addNode("initialization list", initListNodeDecoration);
  builder_.addNode("array", {
    {"shape", "record"}, {"label", "{x} | {...|...}"}});

  builder_.addEdge("reference to a variable", referenceEdgeDecoration);
  builder_.addEdge("initalized with a brace enclosed initializer",
    initListEdgeDecoration);
}

void PointerAnalysisDiagram::decorateNode(
  util::Graph& graph_,
  const util::Graph::Node& node_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setNodeAttribute(node_, attr.first, attr.second);
}

void PointerAnalysisDiagram::decorateEdge(
  util::Graph& graph_,
  const util::Graph::Edge& edge_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setEdgeAttribute(edge_, attr.first, attr.second);
}

void PointerAnalysisDiagram::decorateSubgraph(
  util::Graph& graph_,
  const util::Graph::Subgraph& subgraph_,
  const Decoration& decoration_) const
{
  for (const auto& attr : decoration_)
    graph_.setSubgraphAttribute(subgraph_, attr.first, attr.second);
}

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::globalMemorySubgraphDecoration = {
  {"label", "Global memory"},
  {"style", "dashed"},
  {"color", "#d3d3d3"}
};
const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::stackMemorySubgraphDecoration = {
  {"label", "Stack memory"},
  {"style", "dashed"},
  {"color", "#d3d3d3"}
};
const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::heapMemorySubgraphDecoration = {
  {"label", "Heap memory"},
  {"style", "dashed"},
  {"color", "#d3d3d3"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::centerNodeDecoration = {
  {"style", "filled"},
  {"fillcolor", "gold"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::stackNodeDecoration = {
  {"shape", "Msquare"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::objectNodeDecoration = {
  {"shape", "rect"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::undefinedNodeDecoration = {
  {"shape", "rect"},
  {"style", "filled"},
  {"fillcolor", "red"},
  {"fontcolor", "white"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::nullptrNodeDecoration = {
  {"shape", "rect"},
  {"style", "filled"},
  {"fillcolor", "#ef9c9f"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::functionNodeDecoration = {
  {"shape", "hexagon"},
  {"style", "filled"},
  {"fillcolor", "#ded496"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::initListNodeDecoration = {
  {"shape", "rect"},
  {"color", "#b2c9ec"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::referenceEdgeDecoration = {
  {"style", "dashed"}
};

const PointerAnalysisDiagram::Decoration
PointerAnalysisDiagram::initListEdgeDecoration = {
  {"style", "dashed"},
  {"color", "#d3d3d3"}
};

}
}
}
