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
    const boost::program_options::variables_map& config_);

  void getFunctionCallDiagram(
    util::Graph& graph_,
    const core::AstNodeId& astNodeId_);

private:
  typedef std::map<std::string, std::string> Decoration;

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

  std::map<core::FileId, util::Graph::Subgraph> _subgraphs;

  CppServiceHandler _cppHandler;
  core::ProjectServiceHandler _projectHandler;
};

}
}
}

#endif
