#ifndef CC_SERVICE_LANGUAGE_FILEDIAGRAM_H
#define CC_SERVICE_LANGUAGE_FILEDIAGRAM_H

#include <service/cppservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

namespace cc
{
namespace service
{
namespace language
{

class FileDiagram
{
public:
  FileDiagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  /**
   * This diagram shows the module which directory depends on. The "depends on"
   * diagram on module A traverses the subdirectories of module A and shows all
   * directories that contain files that any of the source files in A includes.
   */
  void getExternalDependencyDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the External dependency diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getExternalDependencyDiagramLegend();

  /**
   * This diagram shows directories (modules) that are users of the
   * queried module.
   */
  void getExternalUsersDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the External users diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getExternalUsersDiagramLegend();

  /**
   * This diagram shows of the `#include` file dependencies.
   */
  void getIncludeDependencyDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the Include dependency diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getIncludeDependencyDiagramLegend();

  /**
   * Interface diagram shows the used and provided interfaces of a source code
   * file and shows linking information.
   */
  void getInterfaceDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the Interface diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getInterfaceDiagramLegend();

  /**
   * This diagram shows the directories relationship between the subdirectories
   * of the queried module. This diagram is useful to understand the
   * relationships of the subdirectories (submodules) of a module.
   */
  void getSubsystemDependencyDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the Subsystem dependency diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getSubsystemDependencyDiagramLegend();

  /**
   * Component users diagram for source file S shows which source files depend
   * on S through the interfaces S provides.
   */
  void getComponentUsersDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  /**
   * This function creates legend for the Component users diagram.
   * @return The generated legend as a string in SVG format.
   */
  std::string getComponentUsersDiagramLegend();

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  /**
   * This function adds a node which represents a File node. The label of the
   * node is the file name.
   */
  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

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
   * Returns the last `n` parts of the path with a '...' if the part size is
   * larger than `n`.
   * (E.g.: path = `/a/b/c/d` and n = `3` then the is result = .../b/c/d)
   */
  std::string getLastNParts(const std::string& path_, std::size_t n_);

  /**
   * This function creates graph nodes for each files which the given one
   * includes.
   * @see getIncludedFiles()
   */
  std::vector<util::Graph::Node> getIncludes(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each files which includes the given
   * file.
   * @note This function is the revert version of the getIncludes function.
   * @see getIncludedFiles()
   */
  std::vector<util::Graph::Node> getRevIncludes(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function returns the graph nodes which the given file includes.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @param reverse_ If true then it creates graph nodes for each file which
   * includes the given file.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getIncludedFiles(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_,
    bool reverse_ = false);

  /**
   * This function creates graph nodes for sub directories of the actual file
   * node and returns the created graph nodes.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @return Subdirectory graph nodes.
   */
  std::vector<util::Graph::Node> getSubDirs(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each files which files implements
   * the given one.
   * @see getImplementedFiles()
   */
  std::vector<util::Graph::Node> getImplements(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each files which implements the given
   * file.
   * @note This function is the revert version of the getImplements function.
   * @see getImplementedFiles()
   */
  std::vector<util::Graph::Node> getRevImplements(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
    * This function creates graph nodes for each files which implements the
    * given file.
    * @note `A` implements `B` if a function which is defined in `A` declared
    * in `B`.
    * @param graph_ A graph object.
    * @param fileNode_ Graph file node object which represents a file object.
    * @param reverse_ If true then it creates graph nodes for each file which
    * implements the given file.
    * @return Created graph nodes.
    */
  std::vector<util::Graph::Node> getImplementedFiles(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_,
    bool reverse_ = false);

  /**
   * This function returns graph nodes which the given file depends on.
   * @see getDependFiles()
   */
  std::vector<util::Graph::Node> getDepends(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function returns graph nodes which depends from the given file.
   * @note This function is the revert version of the getImplements function.
   * @see getDependFiles()
   */
  std::vector<util::Graph::Node> getRevDepends(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function returns graph nodes which the given file depends on.
   * @note File `A` depends on file `B` if:
   *   - `A` has a value declaration which type is a record type which was
   *     declared in `B`.
   *     E.g.: In file A `T x;` is a value declaration and T is a record type
   *           which was declared in file `B` then `A` depends on `B`.
   *   - `A` has a function call which declaration is located in file `B`.
   *     E.g.: In file A `f()` is a function call which function was declared
   *           in file `B` then `A` depends on `B`.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @param reverse_ If true then it creates graph nodes for each file which
   * depends the given file.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getDependFiles(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_,
    bool reverse_ = false);

  /**
   * This function creates graph nodes for each files which files provides
   * the given one.
   * @see getProvidedFiles()
   */
  std::vector<util::Graph::Node> getProvides(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each files which provides the given
   * file.
   * @note This function is the revert version of the getProvides function.
   * @see getProvidedFiles()
   */
  std::vector<util::Graph::Node> getRevProvides(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each files which provides the
   * given file.
   * `A` provides `B` if a function which is defined in `A` , declared in `B`
   * and `A` is not the same as `B`.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @param reverse_ If true then it creates graph nodes for each file which
   * provides the given file.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getProvidedFiles(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_,
    bool reverse_ = false);

  std::vector<core::FileId> getProvidedFileIds(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_,
    bool reverse_ = false);

  /**
   * This function creates graph nodes for each build sources which related to
   * the given build target.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getContains(
    util::Graph& graph_,
    const util::Graph::Node& fileNode_);

  /**
   * This function creates graph nodes for each build targets which related to
   * the given build source.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getRevContains(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This function creates graph nodes for each files which files use the
   * given one.
   * @see getUsedFiles()
   */
  std::vector<util::Graph::Node> getUsages(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This function creates graph nodes for each files which use the given file.
   * @note This function is the revert version of the getUsages function.
   * @see getUsedFiles()
   */
  std::vector<util::Graph::Node> getRevUsages(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This function returns graph nodes which the given file depends on.
   * @note File `A` use file `B` (A<>B) if:
   *   - `A` has a value declaration which type is a record type which was
   *     declared in `B`.
   *     E.g.: In file A `T x;` is a value declaration and T is a record type
   *           which was declared in file `B` then `A` use `B`.
   *   - `A` has a function call which declaration is located in file `B`.
   *     E.g.: In file A `f()` is a function call which function was declared
   *           in file `B` then `A` use `B`.
   * @param graph_ A graph object.
   * @param fileNode_ Graph file node object which represents a file object.
   * @param reverse_ If true then it creates graph nodes for each file which
   * use the given file.
   * @return Created graph nodes.
   */
  std::vector<util::Graph::Node> getUsedFiles(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    bool reverse_ = false);

  std::vector<core::FileId> getUsedFileIds(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    bool reverse_);

  static const Decoration centerNodeDecoration;
  static const Decoration sourceFileNodeDecoration;
  static const Decoration headerFileNodeDecoration;
  static const Decoration binaryFileNodeDecoration;
  static const Decoration objectFileNodeDecoration;
  static const Decoration directoryNodeDecoration;

  static const Decoration usagesEdgeDecoration;
  static const Decoration revUsagesEdgeDecoration;
  static const Decoration providesEdgeDecoration;
  static const Decoration revProvidesEdgeDecoration;
  static const Decoration containsEdgeDecoration;
  static const Decoration revContainsEdgeDecoration;
  static const Decoration subdirEdgeDecoration;
  static const Decoration implementsEdgeDecoration;
  static const Decoration revImplementsEdgeDecoration;
  static const Decoration dependsEdgeDecoration;
  static const Decoration revDependsEdgeDecoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  CppServiceHandler _cppHandler;
  core::ProjectServiceHandler _projectHandler;
};

} // language
} // service
} // cc

#endif // CC_SERVICE_LANGUAGE_FILEDIAGRAM_H
