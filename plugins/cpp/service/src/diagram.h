#ifndef CC_SERVICE_LANGUAGE_DIAGRAM_H
#define CC_SERVICE_LANGUAGE_DIAGRAM_H

#include <service/cppservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

namespace cc
{
namespace service
{
namespace language
{

class Diagram
{
public:
  Diagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const boost::program_options::variables_map& config_);

  void getFunctionCallDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

  /**
   * This function creates legend for the Function call diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getFunctionCallLegend();

  /**
   * This is a classical UML class diagram for the selected class and its
   * direct children and parents. The nodes contain the methods and member
   * variables with their visibility.
   */
  void getDetailedClassDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

  /**
   * This function creates legend for the Detailed class diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getDetailedClassLegend();

  /**
   * This diagram for a class shows recursively the related classes and their
   * inheritance and containment relationships.
   */
  void getClassCollaborationDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

  /**
   * This function creates legend for the Class collaboration diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getClassCollaborationLegend();

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;
  typedef std::pair<util::Graph::Node, util::Graph::Node> GraphNodePair;

  /**
   * This function adds a node which represents an AST node. The label of the
   * node is the AST node value. A node associated with the file is added only
   * once. If already added then the previous one is returned.
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
   * This function creates node label for UML class diagram for the
   * selected class.
   */
  std::string getDetailedClassNodeLabel(const AstNodeInfo& nodeInfo_);

  /**
   * This function return string representation visibility of an AST node
   * in HTML format.
   */
  std::string visibilityToHtml(const AstNodeInfo& node_);

  /**
   * This function returns member content styled by their properties.
   * (E.g.: static -> underline, virtual -> italic etc.)
   */
  std::string memberContentToHtml(
    const AstNodeInfo& node_,
    const std::string& content_);

  /**
   * This function returns the property of an AST node.
   */
  std::string getProperty(
    const core::AstNodeId& astNodeId_,
    const std::string& property_);

  /**
   * This function decorates a graph element.
   * @param graph_ A graph object.
   * @param elem_ A graph element: node, edge or subgraph.
   * @param decoration_ A map which describes the style attributes.
   */
  template <typename T>
  void decorate(
    util::Graph& graph_,
    const T& elem_,
    const Decoration& decoration_) const
  {
    for (const auto& attr : decoration_)
      graph_.setAttribute(elem_, attr.first, attr.second);
  }

  static const Decoration centerNodeDecoration;
  static const Decoration calleeNodeDecoration;
  static const Decoration callerNodeDecoration;
  static const Decoration virtualNodeDecoration;
  static const Decoration calleeEdgeDecoration;
  static const Decoration callerEdgeDecoration;
  static const Decoration centerClassNodeDecoration;
  static const Decoration classNodeDecoration;
  static const Decoration usedClassEdgeDecoration;
  static const Decoration inheritClassEdgeDecoration;

  std::map<core::FileId, util::Graph::Subgraph> _subgraphs;

  CppServiceHandler _cppHandler;
  core::ProjectServiceHandler _projectHandler;
};

}
}
}

#endif
