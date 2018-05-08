#ifndef CC_SERVICE_CPPREPARSESERVICE_INFOTREE_H
#define CC_SERVICE_CPPREPARSESERVICE_INFOTREE_H

#include <map>
#include <memory>
#include <string>

#include <boost/optional.hpp>

#include <model/cppastnode.h>

// Required for the Thrift objects, such as core::FileId.
#include "cppreparse_types.h"

#include "astcache.h"

namespace cc
{
namespace service
{
namespace language
{

/**
 * This class can be used to produce InfoTree-like nodes from an AST that can be
 * rendered for the user.
 */
class ASTInfoTree
{
public:
  struct Node
  {
    /**
     * The index of the node based on the Clang AST Visitor visition order.
     */
    size_t _visitId;

    /**
     * The memory address of the node itself.
     */
    void* _clangMemoryAddress;

    /**
     * The extracted (not necessarily the most accurate) type of the node.
     */
    reparse::ClangASTNodeType _nodeType;

    /**
     * Whether the node has further children in the tree. This value is always
     * properly populated at data extraction, and does not use lazy traversal.
     */
    bool _hasChildren;
  };

  /**
   * Instantiate a node detail extraction for the given file and the AST Context
   * that is generated from reparsing the file.
   */
  // TODO: Yet again, what would you do with headers? (Later...)
  ASTInfoTree(reparse::ASTCache* cache_,
              const core::FileId& fileId_,
              clang::ASTContext& context_)
    : _fileId(fileId_),
      _astCache(cache_),
      _context(context_)
  {
    assert(cache_ && "AST InfoTree creation depends on a cache to speed up "
                     "and type lookups.");
  }

  /**
   * Retrieve the basic data for the given database-backed ASTNode by searching
   * the AST stored in the current class.
   */
  boost::optional<Node> getNodeDataForAstNode(
    model::CppAstNodePtr astNode_);

  /**
   * Retrieve the basic data for the given visit-ordered node ID by traversing
   * and numbering the AST nodes from the top.
   */
  boost::optional<Node> getNodeDataForId(size_t nodeId_);

  /**
   * Retrieve the basic representation information for the children of the
   * given node.
   */
  std::vector<Node> getChildrenForNode(Node node_);

  /**
   * Retrieve the full type of the Clang AST Node (such as CXXNewExpr,
   * RecordDecl) for the Node record given.
   */
  std::string getNodeType(Node node_);

  // TODO: Methods for further requesting some information from the AST.

  /**
   * Get some detailed information about the Node given.
   * Currently this returns the full line of the "-ast-dump" of the node itself.
   */
  std::string dummyGetDetails(Node node_);

private:
  const core::FileId& _fileId;
  reparse::ASTCache* _astCache;
  clang::ASTContext& _context;
};

} //namespace language
} //namespace service
} //namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_INFOTREE_H
