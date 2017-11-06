#ifndef CC_SERVICE_LANGUAGE_POINTERANALYSISDIAGRAM_H
#define CC_SERVICE_LANGUAGE_POINTERANALYSISDIAGRAM_H

#include <service/cppservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

#include <model/cpppointeranalysis.h>

#include <util/legendbuilder.h>

namespace cc
{
namespace service
{
namespace language
{

class PointerAnalysisDiagram
{
public:
  PointerAnalysisDiagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const boost::program_options::variables_map& config_);

  /**
   * Generate pointer analysis diagram for Andersen style points-to analysis.
   */
  void getAndersenPointerAnalysisDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

  /**
   * Generate pointer analysis diagram for Steensgaard style points-to analysis.
   */
  void getSteensgaardPointerAnalysisDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

  /**
    * This function creates legend for Andersen style points-to analysis
    * diagram.
    * @return The generated legend as a string in SVG format.
    */
  std::string getAndersenDiagramLegend();

  /**
    * This function creates legend for the Steensgaard style points-to analysis
    * diagram.
    * @return The generated legend as a string in SVG format.
    */
  std::string getSteensgaardDiagramLegend();

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  /**
    * This function creates common legend for Andersen and Steensgaard style
    * points-to analysis diagram.
    */
  void getPointerAnalysisDiagramLegend(util::LegendBuilder& builder_);

  /**
   * Creates subgraphs for memory modeling: `heap`, `stack`, `global`.
   */
  void createMemorySubraphs(util::Graph& graph_);

  /**
   * Get AST node from cache or from the database.
   */
  bool getAstNode(
    std::uint64_t mangledNameHash_,
    std::unordered_map<std::uint64_t, model::CppAstNode>& astNodeCache_);

  /**
   * Creates pointer analysis node.
   * @param addToSubraphs_ If true it will create graph node in the correct
   * subgraph which determined by the options_ parameter.
   */
  void createPointerAnalysisNode(
    util::Graph& graph_,
    const std::string& nodeId_,
    model::CppPointerAnalysis::Options_t options_,
    bool addToSubraphs_ = false);

  /**
   * Decorate pointer analysis graph edge.
   */
  void decoratePointerAnalysisEdge(
    util::Graph& graph_,
    const util::Graph::Edge& edge_,
    model::CppPointerAnalysis::Options_t lhsOptions_,
    model::CppPointerAnalysis::Options_t rhsOptions_);

  /**
   * Decorate pointer analysis graph node.
   */
  void decoratePointerAnalysisNode(
    util::Graph& graph_,
    const model::CppAstNode& astNode,
    model::CppPointerAnalysis::Options_t options_);

  /**
   * This function decorates a graph node.
   * @param graph_ A graph object.
   * @param elem_ A graph node
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateNode(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

  /**
   * This function decorates a graph edge.
   * @param graph_ A graph object.
   * @param elem_ A graph edge
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateEdge(
    util::Graph& graph_,
    const util::Graph::Edge& edge_,
    const Decoration& decoration_) const;

  /**
   * This function decorates a graph subgraph.
   * @param graph_ A graph object.
   * @param elem_ A graph subgraph
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateSubgraph(
    util::Graph& graph_,
    const util::Graph::Subgraph& subgrap_,
    const Decoration& decoration_) const;

  static const Decoration centerNodeDecoration;
  static const Decoration stackNodeDecoration;
  static const Decoration objectNodeDecoration;
  static const Decoration undefinedNodeDecoration;
  static const Decoration nullptrNodeDecoration;
  static const Decoration functionNodeDecoration;
  static const Decoration initListNodeDecoration;

  static const Decoration globalMemorySubgraphDecoration;
  static const Decoration stackMemorySubgraphDecoration;
  static const Decoration heapMemorySubgraphDecoration;

  static const Decoration referenceEdgeDecoration;
  static const Decoration initListEdgeDecoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  CppServiceHandler _cppHandler;
  core::ProjectServiceHandler _projectHandler;
};

} // language
} // service
} // cc

#endif // CC_SERVICE_LANGUAGE_POINTERANALYSISDIAGRAM_H
