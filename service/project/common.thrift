namespace cpp cc.service.core
namespace java cc.service.core

/**********************
 * General Exceptions *
 **********************/

exception DatasourceError
{
  1:string what,
  2:i64 errorcode
}

exception LongWaitDiagramEx
{
  1:string what
}

exception InvalidInput
{
  1:string what
}

/************************
 * Identifier structures *
 ************************/

struct AstNodeId
{
  1:string astNodeId
}

struct ProjectId
{
  1:i64 prid
}

struct FileId
{
  1:string fid
}

exception InvalidId
{
  1:string what,
  2:i64 prid,
  3:string fid
}

/******************************
 * File positions & locations *
 ******************************/

struct Position
{
  1:i32 line   = -1,
  2:i32 column = -1
}

struct Range
{
  1:Position startpos,
  2:Position endpos
}

struct SyntaxHighlight{
    1: Range range,
    3: string className
}

struct FilePosition
{
  1:FileId file,
  2:Position pos
}

struct FileRange
{
  1:FileId file,
  2:Range range
}

exception InvalidPosition
{
  1:string what,
  2:FilePosition fpos
}

exception InvalidRange
{
  1:string what,
  2:FileRange frange
}

/****************************
 *References and diagram    *
 *types                     *
 ***************************/

enum DiagramId {
  FUNCTION_CALL,
  CLASS,
  FULL_CLASS,
  DIR_FULL_CLASS,
  CLASS_COLLABORATION,
  FUNCTION_PATH,
  INCLUDE_DEPENDENCY,
  CODE_BITES,
  INTERFACE,
  COMPONENTS_USED,
  COMPONENT_USERS,
  SUBSYSTEM_IMPLEMENT,
  SUBSYSTEM_DEPENDENCY,
  EXTERNAL_IMPLEMENTS,
  EXTERNAL_DEPENDENCIES,
  EXTERNAL_USERS,
  MI_NEIGHBOUR_CONNECTIONS,
  MI_DEPENDENCY,
  MI_USAGE,
  MI_DELOS,
  DELOS_DATA,//Persistent object UML class diagram
  MI_EDITOR_DIAGRAM,
  POINTER_ANALYSIS,
  MI_HIERARCHY_DIAGRAM
}

struct DiagramType{
  1:string    diagramName, /** Unique human readable name of the diagram that the GUI can print out */
  2:DiagramId diagramId    /** Unique id of a diagram type within the scope of a language */
}

struct ReferenceType{
  1:string referenceName, /** Unique human readable name of the reference that the GUI can print out */
  2:i64    referenceId    /** Unique id of a reference type within the scope of a language */
}

/**
* the basic menu type categories
* these determine which display method will be used, and which thrift function will be called to obtain data
*/
enum Category {
  references   = 1, /** getReferences called and in the query result accordion dipslayed */
  infoTree     = 2, /** getInfoTree called, and in the info tree accordion displayed */
  diagram      = 3, /** getDiagram called, and as a diagram displayed */
  pagingResult = 4, /** getPage called, and in the query result accordion displayed */
  codebites    = 5, /** CodeBites diagram displayed */
  slicing      = 6, /** Select variables by slicing */
  jumpToDef    = 7, /** Jump to definition */
  infoBox      = 8, /** Display info box */
}

struct MenuType{
  1:list<string> name,       /** A list of strings representing the path to this element in the menu selection tree, last item is the name */
  2:Category     category,   /** Unique id of a menu item type within the scope of a language */
  3:i64          menuItemId, /** Unique id of a menu item within the scope of a language */
  4:string       helpText,   /** Information text about this menu type */
  5:string       shortcut,   /** A string indicating keyboard shortcut (e.g. ctrl - click) */
  6:AstNodeId    astNodeId,  /** AstNodeId of a node to which the menu item refers */
}



