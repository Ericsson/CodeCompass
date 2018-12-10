include "project/common.thrift"
include "project/project.thrift"

namespace cpp cc.service.language

// TODO: Throw exception if file ID or ASTNode ID is invalid.

// TODO: Do something about header files and AST Nodes in headers. Currently
// there is no build action for the headers and thus the whole reparse can't
// work for them.

/**
 * To make sure the InfoTree AST view is as lazy-loading as possible, the
 * information contained therein is split into two types. This one is the basic
 * smaller type that is used to render a node without any unnecessary extra
 * details.
 */
struct ASTNodeBasic
{
  1: i64    visitId /** The visit-order of the node, relative to the root node of the tree being traversed. */
  2: string type /** The type name of the AST node (e.g. FunctionDecl). */
  3: bool   hasChildren /** Whether the node has further nodes as children. */
}

/**
 * Contains all the other extra information that is usually loaded when the
 * details of an InfoTree AST is expanded.
 */
struct ASTNodeDetail
{
  // TODO: Expand 1 to be more detailed, typed, etc. Currently this sends only a dump.
  1: string otherStuff /** The other things the AST node can be described with. */
  2: list<ASTNodeBasic> children /** Basic details about the children nodes. */
}

service CppReparseService
{
  /**
  * Returns false if the server was started with a command-line argument that
  * disables the reparse capabilities.
  */
  bool isEnabled();

  /**
   * Returns the Abstract Syntax Tree (AST) for the given file as HTML string.
   */
  string getAsHTML(1: common.FileId fileId);

  /**
   * Returns the AST for the given AST Node('s subtree) as an HTML string.
   */
  string getAsHTMLForNode(1: common.AstNodeId nodeId);
}
