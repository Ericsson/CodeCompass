include "project/common.thrift"
include "project/project.thrift"
include "project/result.thrift"

namespace cpp cc.service.language

struct AstNodeInfo
{
  1:common.AstNodeId astNodeId, /**< Uniqe id of the AST node for the whole workspace */
  2:string astNodeType, /**< string representation of AST type (e.g. Function/Type/Variable) */
  4:string astNodeValue, /**< string representation of an AST node */
  6:string astNodeSrcText /**< corresponding code fragment in the source code */
  8:common.FileRange range, /**< source code range of an AST node */
  12:string documentation /**< documentation of an AST node  */
}

struct InfoQuery
{
  1:i32 queryId, /**< id of query, e.g. id of "get callers" or "get calls" etc.
  					  zero ID indicates absent InfoQuery */
  2:list<string> filters /**< query filters */
}

struct InfoNode
{
  1:list<string> category, /**< the hierarchy in list */
  2:string label, /**< the description of the field (optional) */
  4:string value, /**< non-clickable value (optional) */
  8:AstNodeInfo astValue, /**< clickable value (optional) */
  12:InfoQuery query /**< to query a subTree lazily (optional) */
}

struct InfoBox
{
  1:string information, /**< the information to show */
  2:project.FileType fileType, /**< fileType for proper highlighting */
  16:string documentation
}

enum RefTypes
{
  GetDef,
  GetUsage,
  GetFuncCalls,
  GetCallerFuncs,
  GetAssignsToFuncPtrs,
  GetGeneratedCodeCpp,
  GetGeneratedCodeJava
}

enum SlicingTypes
{
  Before,
  After
}

service LanguageService
{
    
  /**
  * Returns an AstNodeInfo object for the given source code position
  * @param fpos file position in the source file
  * @param filters resolve ambigous result (e.g. in case of c++ the current .so or .o
  * can be specified here)
  * @return the AstNodeInfo object
  */
  AstNodeInfo getAstNodeInfoByPosition(1:common.FilePosition fpos, 2:list<string> filters)
    throws (1:common.InvalidId ex, 2:common.InvalidPosition posex)
    
  /**
  * Returns an InfoBox object for the given AST node.
  * @param astNodeId ID of the given AST node.
  * @return the InfoBox object
  */
  InfoBox getInfoBox(1:common.AstNodeId astNodeId)
    throws (1:common.InvalidId ex)

  /**
  * Returns an InfoBox object for the given source code position
  * @param fpos file position in the source file
  * @param filters resolve ambigous result (e.g. in case of c++ the current .so or .o
  * can be specified here)
  * @return the InfoBox object
  */
  InfoBox getInfoBoxByPosition(1:common.FilePosition fpos, 2:list<string> filters)
    throws (1:common.InvalidId ex, 2:common.InvalidPosition posex)

  /**
  * Returns an AstNodeInfo object for the given AST node id
  * @astNodeId ID of an AST node
  * @return the AstNodeInfo object
  */
  AstNodeInfo getAstNodeInfo(1:common.AstNodeId astNodeId)


  /**
  * Returns the graphviz svg represenation of a diagram about astnode identified by astNodeId and diagarm type identified by diagramId
  * @param astNodeId the astNode we want to draw diagram of  
  * @param diagramId the diagram type we want to draw
  * @return Graphviz svg represenation of the diagram
  */
  string getDiagram(1:common.AstNodeId astNodeId,2:common.DiagramId diagramId);


  /**
  * Returns the svg represenation of the diagram legend used by getDiagram()
  * @param diagramId the diagram type
  * @return Svg represenation of the diagram legend
  */
  string getLegend(1:common.DiagramId diagramId);

  /**
  * Returns the graphviz svg representation of a diagram about the paths between two functions
  * @param astNodeIdFrom the starting function
  * @param astNodeIdTo the ending function
  * @return Graphviz svg representation of the diagram
  */
  string getFunctionCallPathDiagram(1:common.AstNodeId astNodeIdFrom,2:common.AstNodeId astNodeIdTo);

  /**
  * Returns references to the AST node identified by astNodeId and query type identified by referenceId
  * @param astNodeId the astNode to be queried  
  * @param referenceId reference type (such as derivedClasses, definition, usages etc.)
  * @return list of references
  */
  list<AstNodeInfo> getReferences(1:common.AstNodeId astNodeId,2:RefTypes referenceId);
  
  /**
  * Returns references to the AST node identified by astNodeId and query type identified by referenceId
  * @param astNodeId the astNode to be queried  
  * @param referenceId reference type (such as derivedClasses, definition, usages etc.)
  * @param fileId id of the file in which we search for the references
  * @return list of references
  */
  list<AstNodeInfo> getReferencesInFile(1:common.AstNodeId astNodeId,2:RefTypes referenceId, 3:common.FileId fileId);
  
  /**
  * Returns a list which contains file names and match counts
  * @param astNodeId the astNode to be queried
  * @param referenceId reference type (such as derivedClasses, definition, usages etc.)
  * @param pageSize the row count of the page to display
  * @param pageNo the number of the page to display
  * @return list of file names and match counts
  */
  result.RangedHitCountResult getPage(1:common.AstNodeId astNodeId, 2:RefTypes referenceId,
    3:i32 pageSize, 4:i32 pageNo);
 
/**
  * Returns a list of diagram types that can be drawn for the specified file
  * @param fileId the file Id we would like to draw the diagram of
  * @return list of supported diagram types (such as dependency)
  */
  //list<common.DiagramType> getFileDiagramTypes(1:common.FileId fileId);

  /**
  * Returns a Graphviz dot language representation of the required diagram graph
  * @param fileId the file Id we would like to draw the diagram of  
  * @param diagramId the diagram type we want to draw
  * @return phviz dot represenation of the diagram 
  */
  string getFileDiagram(1:common.FileId fileId,2:common.DiagramId diagramId)
    throws(1:common.LongWaitDiagramEx ex)

  /**
  * Returns a list of reference types that can listed for the requested file (such as includes, included by)
  * @param fileId the file Id we want to get the references about 
  * @return list of supported reference types
  */
  //list<common.ReferenceType> getFileReferenceTypes(1:common.FileId fileId);

  /**
  * Returns references as an answer to the requested search
  * @param fileId the file Id we want to get the references about 
  * @param common.ReferenceType we want (such as includes,includedby etc)
  * @return list of references
  */
  list<AstNodeInfo> getFileReferences(1:common.FileId fileId,2:RefTypes referenceId);

  /**
  * Return a list of ast node ids to type definitions under a file path
  *
  * @param the path
  * @return a list of ast node id to type definitions
  */
  list<common.AstNodeId> getTypeDefinitions(1:string path);

  /**
   * Returns a recursive list of InfoNodes
   * @param astNodeId the ast node we want to get information
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getInfoTree(1:common.AstNodeId astNodeId);
  
  /**
   * Returns a recursive list of InfoNodes
   * @param astNodeId the ast node we want to get information
   * @param query determines what info we are interested in
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getSubInfoTree(1:common.AstNodeId astNodeId, 2:InfoQuery query);
  
  /**
   * Returns a recursive list of InfoNodes
   * 
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getCatalogue();
  
  /**
   * Returns a recursive list of InfoNodes
   * @param query determines what info we are interested in
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getSubCatalogue(1:InfoQuery query);
  
  /**
   * Returns a recursive list of InfoNodes
   * @param fileId the file we want to get information
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getInfoTreeForFile(1:common.FileId fileId);
  
  /**
   * Returns a recursive list of InfoNodes
   * @param fileId the file node we want to get information
   * @param query determines what info we are interested in
   * @return list of possible recursive information nodes
   */
  list<InfoNode> getSubInfoTreeForFile(1:common.FileId fileId, 2:InfoQuery query);
  
  /**
  * Returns a list of supported menu types for AST nodes, 
  * the list will be displayed to the user a menu, and its elements are actions supported by the server
  * @param astNodeId the astNode to be queried
  * @return list of supported menu items (such as references derived classes,
  * definition, usages etc., diagrams, info tree).
  */
  list<common.MenuType> getMenuTypes(1:common.AstNodeId astNodeId);

  /**
   * Returns a list of supported menu types for files
   * the list will be displayed to the user a menu, and its elements are actions supported by the server
   * @param fileId the file id to be queried
   * @return list of supported menu items
   */
  list<common.MenuType> getFileMenuTypes(1:common.FileId fileId);

  /**
   * Returns a list of supported menu types for directories
   * the list will be displayed to the user a menu, and its elements are actions supported by the server
   * @param dirId the directory id to be queried
   * @return list of supported menu items
   */
  list<common.MenuType> getDirMenuTypes(1:common.FileId dirId);
  
  /**
  * Returns the source code for the given astNode
  * @param astNodeId the astNode to be queried
  * @return source code for the given astNode
  */
  string getSourceCode(1:common.AstNodeId astNodeId);

  /**
   * Returns the documentation comment for the given astNode, if present, in
   * HTML format
   * @param astNodeId the astNode to be queried
   * @return the documentation comment for the given astNode, empty string
   * if not available.
   */
  string getDocComment(1:common.AstNodeId astNodeId);

  list<common.Range> getBackwardSlicePos(1:common.FilePosition filePos);
  list<common.Range> getForwardSlicePos(1:common.FilePosition filePos);

   /**
   * Returns the syntax highlight elements
   * @param fileId id of the file in which we get syntax elements
   * @return elements position and css class name
   */
  list<common.SyntaxHighlight> getSyntaxHighlight(1:common.FileId fileId);
}
