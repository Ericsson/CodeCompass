include "../project/common.thrift"
include "../project/project.thrift"

namespace cpp cc.service.language

struct AstNodeInfo
{
  1:common.AstNodeId id /** Unique id of the AST node for the whole workspace. */
  2:i64    mangledNameHash /** To resolve unique names of the AST node. */
  3:string astNodeType /** String representation of AST type (e.g. Statement/Declaration/Usage). */
  4:string symbolType /** String representation of Symbol type (e.g. Function/Type/Variable). */
  5:string astNodeValue /** String representation of an AST node. */
  6:string srcText /** Corresponding code fragment in the source code. */
  7:common.FileRange range /** Source code range of an AST node. */
  8:list<string> tags /** Meta information of the AST node (e.g. public, static, virtual etc.) */
}

struct SyntaxHighlight
{
  1:common.Range range, /** Source code range of an AST node. */
  2:string className /** CSS class name */
}

service LanguageService
{
  /**
   * Return the file types which can be used to associate
   * the file types with the service
   * @return File types
   */
  list<string> getFileTypes()

  /**
   * Returns an AstNodeInfo object for the given AST node ID.
   * @param astNodeId ID of an AST node.
   * @return The corresponding AstNodeInfo object.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  AstNodeInfo getAstNodeInfo(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
   * Returns an AstNodeInfo object for the given source code position.
   * @param fpos File position in the source file.
   * @return The AstNodeInfo object at the given position. If more AST nodes are
   * found at the given position nested in each other (e.g. in a compound
   * expression) then the innermost is returned.
   * @exception common.InvalidInput Exception is thrown if no AST node found
   * at the given position.
   */
  AstNodeInfo getAstNodeInfoByPosition(1:common.FilePosition fpos)
    throws (1:common.InvalidInput ex)

  /**
   * Returns the documentation which belongs to the given AST node if any
   * (Doxygen, Python doc, etc.).
   * @param astNodeId ID of an AST node.
   * @return The documentation of the given node.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  string getDocumentation(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
   * Returns a set of properties which can be known about the given AST node.
   * @param astNodeId ID of an AST node.
   * @return A collection which maps the property name to the property value.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  map<string, string> getProperties(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
   * Returns the diagram types which can be passed to getDiagram() function for
   * the given AST node.
   * @param astNodeId ID of an AST node.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  map<string, i32> getDiagramTypes(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
   * Returns the SVG represenation of a diagram about the AST node identified by
   * astNodeId and diagarm type identified by diagramId.
   * @param astNodeId The AST node we want to draw diagram about.
   * @param diagramId The diagram type we want to draw. The diagram types can be
   * queried by getDiagramTypes().
   * @return SVG represenation of the diagram. If the diagram can't be generated
   * then empty string returns.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   * @exception common.Timeout Exception is thrown if the diagram generation
   * times out.
   */
  string getDiagram(1:common.AstNodeId astNodeId, 2:i32 diagramId)
    throws (1:common.InvalidId exId, 2:common.Timeout exLong)

  /**
   * Returns the SVG represenation of the diagram legend used by getDiagram().
   * @param diagramId The diagram type. This should be one of the IDs returned
   * by getDiagramTypes().
   * @return SVG represenation of the diagram legend or empty string if the
   * legend can't be generated.
   */
  string getDiagramLegend(1:i32 diagramId)

  /**
   * Returns a list of diagram types that can be drawn for the specified file.
   * @param fileId The file ID we would like to draw the diagram about.
   * @return List of supported diagram types (such as dependency).
   * @exception common.InvalidId Exception is thrown if no file belongs to the
   * given ID.
   */
  map<string, i32> getFileDiagramTypes(1:common.FileId fileId)
    throws (1:common.InvalidId ex)

  /**
   * Returns an SVG representation of the required diagram graph.
   * @param fileId The file ID we would like to draw the diagram aboue.
   * @param diagramId The diagram type we want to draw. These can be queried by
   * getFileDiagramTypes().
   * @return SVG represenation of the diagram.
   * @exception common.InvalidId Exception is thrown if no ID belongs to the
   * given fileId.
   * @exception common.Timeout Exception is thrown if the diagram generation
   * times out.
   */
  string getFileDiagram(1:common.FileId fileId, 2:i32 diagramId)
    throws (1:common.InvalidId exId, 2:common.Timeout exLong)

  /**
   * Returns the SVG represenation of the diagram legend used by
   * getFileDiagram().
   * @param diagramId The diagram type. This should be one of the IDs returned
   * by getFileDiagramTypes().
   * @return SVG represenation of the diagram legend or empty string if the
   * legend can't be generated.
   */
  string getFileDiagramLegend(1:i32 diagramId);

  /**
   * Returns the reference types which can be passed to getReferences().
   * @param astNodeId ID of an AST node.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  map<string, i32> getReferenceTypes(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
   * Returns references to the AST node identified by astNodeId.
   * @param astNodeId The AST node to be queried.
   * @param referenceId Reference type (such as derivedClasses, definition,
   * usages etc.). Possible values can be queried by getReferenceTypes().
   * @param tags Meta-information which can help to filter query results of
   * the AST node (e.g. public, static)
   * @return List of references.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  list<AstNodeInfo> getReferences(
    1:common.AstNodeId astNodeId,
    2:i32 referenceId
    3:list<string> tags)
      throws (1:common.InvalidId ex)

  /**
   * Returns references to the AST node identified by astNodeId restricted to a
   * given file. Sometimes (e.g. in a GUI) it is sufficient to list only the
   * results in a file, and this may make the implementation faster.
   * @param astNodeId The astNode to be queried.
   * @param referenceId reference type (such as derivedClasses, definition,
   * usages etc.).
   * @param fileId ID of the file in which we search for the references.
   * @param tags Meta-information which can help to filter query results of
   * the AST node (e.g. public, static)
   * @return List of references.
   * @exception common.InvalidId Exception is thrown if not AST node or file
   * belongs to the given IDs.
   */
  list<AstNodeInfo> getReferencesInFile(
    1:common.AstNodeId astNodeId,
    2:i32 referenceId,
    3:common.FileId fileId
    4:list<string> tags)
      throws (1:common.InvalidId ex)

  /**
   * Same as getReferences() but only a few results are returned based on the
   * parameters.
   * @param astNodeId The AST node to be queried.
   * @param referenceId Reference type (such as derivedClasses, definition,
   * usages etc.). Possible values can be queried by getReferenceTypes().
   * @param pageSize The maximum size of the returned list.
   * @param pageNo The number of the page to display, starting from 0.
   * @return List of references.
   * @exception common.InvalidId Exception is thrown if no AST node belongs to
   * the given ID.
   */
  list<AstNodeInfo> getReferencesPage(
    1:common.AstNodeId astNodeId,
    2:i32 referenceId,
    3:i32 pageSize,
    4:i32 pageNo)
      throws (1:common.InvalidId ex)
 
  /**
   * Returns a list of reference types that can be listed for the requested file
   * (such as includes, included by, etc.).
   * @param fileId The file ID we want to get the references about.
   * @return List of supported reference types.
   * @exception common.InvalidId Exception is thrown if no file belongs to the
   * given ID.
   */
  map<string, i32> getFileReferenceTypes(1:common.FileId fileId)
    throws (1:common.InvalidId ex)

  /**
   * Returns references as an answer to the requested search.
   * @param fileId the file ID we want to get the references about.
   * @param referenceType Reference type (e.g. includes, provides, etc.).
   * Possible values can be queried by getFileReferenceTypes().
   * @return List of referenced files.
   * @exception common.InvalidId Exception is thrown if no file belongs to the
   * given ID.
   */
  list<project.FileInfo> getFileReferences(
    1:common.FileId fileId,
    2:i32 referenceId)
      throws (1:common.InvalidId ex)

  /**
   * Returns the syntax highlight elements for a whole file.
   * @param fileId ID of the file in which we get syntax elements.
   * @return Elements' position and CSS class name.
   * @exception common.InvalidId Exception is thrown if no file belongs to the
   * given ID.
   */
  list<SyntaxHighlight> getSyntaxHighlight(1:common.FileId fileId)
    throws (1:common.InvalidId ex)
}
