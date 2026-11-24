#ifndef CC_SERVICE_LANGUAGE_CSHARPDIAGRAM_H
#define CC_SERVICE_LANGUAGE_CSHARPDIAGRAM_H

#include <service/csharpservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

namespace cc
{
namespace service
{
namespace language
{

class CsharpDiagram
{
public:
  CsharpDiagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  /**
   * In the function call diagram the nodes are methods and the edges are the
   * calls between them.
   */
  void getFunctionCallDiagram(
    util::Graph& graph_,
    const AstNodeInfo& centerNodeInfo_,
    const std::vector<AstNodeInfo>& calleeNodeInfos_,
    const std::vector<AstNodeInfo>& callerNodeInfos_);

  /**
   * This function creates legend for the Function call diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getFunctionCallLegend();

  /**
   * This is a classical UML class diagram for the selected type and its direct
   * base types and derived types. The nodes contain the data members and the
   * methods with their visibility. The members are queried through the
   * service handler like in the C++ plugin.
   */
  void getDetailedClassDiagram(
    util::Graph& graph_,
    CsharpServiceHandler& csharpHandler_,
    const core::AstNodeId& astNodeId_);

  /**
   * This function creates legend for the Detailed class diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getDetailedClassLegend();

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  /**
   * This function adds a type node with an UML class label to the graph.
   */
  util::Graph::Node addClassNode(
    util::Graph& graph_,
    CsharpServiceHandler& csharpHandler_,
    const AstNodeInfo& nodeInfo_);

  /**
   * This function creates the HTML label of a type node: the name of the type,
   * its data members (fields and properties) and its methods.
   */
  std::string getDetailedClassNodeLabel(
    CsharpServiceHandler& csharpHandler_,
    const AstNodeInfo& nodeInfo_);

  /**
   * This function returns the colored UML visibility sign of a member based on
   * its tags.
   */
  std::string visibilityToHtml(const AstNodeInfo& node_);

  /**
   * This function makes the already escaped content of a member bold if it is
   * static and italic if it is virtual, abstract or override.
   */
  std::string memberContentToHtml(
    const AstNodeInfo& node_,
    const std::string& content_);

  /**
   * This function adds a node which represents an AST node. The label of the
   * node is the AST node value. A node associated with the AST node is added
   * only once. If already added then the previous one is returned.
   */
  util::Graph::Node addNode(
    util::Graph& graph_,
    const AstNodeInfo& nodeInfo_);

  /**
   * This function adds a subgraph which represents a file. The label of the
   * subgraph will be the file path and the subgraph will have a border. A
   * subgraph associated with the file is added only once. If already added then
   * the previous one is returned.
   */
  util::Graph::Subgraph addSubgraph(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function decorates a graph node.
   * @param graph_ A graph object.
   * @param node_ A graph node
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateNode(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

  /**
   * This function decorates a graph edge.
   * @param graph_ A graph object.
   * @param edge_ A graph edge
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateEdge(
    util::Graph& graph_,
    const util::Graph::Edge& edge_,
    const Decoration& decoration_) const;

  /**
   * This function decorates a graph subgraph.
   * @param graph_ A graph object.
   * @param subgraph_ A graph subgraph
   * @param decoration_ A map which describes the style attributes.
   */
  void decorateSubgraph(
    util::Graph& graph_,
    const util::Graph::Subgraph& subgraph_,
    const Decoration& decoration_) const;

  static const Decoration centerNodeDecoration;
  static const Decoration calleeNodeDecoration;
  static const Decoration callerNodeDecoration;
  static const Decoration calleeEdgeDecoration;
  static const Decoration callerEdgeDecoration;
  static const Decoration classNodeDecoration;
  static const Decoration inheritClassEdgeDecoration;

  std::map<core::FileId, util::Graph::Subgraph> _subgraphs;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  core::ProjectServiceHandler _projectHandler;
};

}
}
}

#endif
