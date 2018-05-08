#include <algorithm>
#include <functional>
#include <stack>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <model/cppastnode-odb.hxx>

#include <util/logutil.h>
#include <util/util.h>

#include <cppparser/filelocutil.h>

#include "astnodelocator.h"
#include "infotree.h"

using namespace cc::service::reparse;

namespace
{

using namespace cc;
using namespace cc::service::language;
using namespace clang;

typedef std::map<size_t, ASTInfoTree::Node> NodeVisitMap;

/**
 * This RecursiveASTVisitor walks the AST and labels the nodes in the order
 * they are visited with an ID, searching for a particular node based on a
 * logic implemented by child classes.
 */
class NodeSearch
  : public ASTConsumer,
    public RecursiveASTVisitor<NodeSearch>
{
  typedef RecursiveASTVisitor<NodeSearch> Base;

public:
  /**
   * This is an abstract class which should not be instantiated by itself.
   */
  NodeSearch() = delete;
  virtual ~NodeSearch() = default;

  bool shouldVisitImplicitCode() const { return true; }
  bool shouldVisitTemplateInstantiations() const { return true; }

  /**
   * Initiate the traversal of the AST. This method must be the one called by
   * the client code with the same context_ as the one that was passed to the
   * constructor.
   */
  void HandleTranslationUnit(ASTContext& context_) override
  {
    assert(&_context == &context_ && "AST visitor must be called with the same "
      "ASTContext as it was set up with.");

    TranslationUnitDecl* d = context_.getTranslationUnitDecl();
    _visitCount = 0;
    TraverseDecl(d);
  }

  /* The traverse methods first walk the AST until the searched node is found.
   * The located node's ID is saved, and then the traversal continues on the
   * subtree of this node, labelling every node with its index still. This is
   * used to then gather which IDs belong to the direct children of the node.
   */

  bool TraverseDecl(Decl* d_)
  {
    return TraverseClangASTNode(d_, [&]{ return Base::TraverseDecl(d_); },
                                std::mem_fn(&NodeSearch::tryLocateDecl),
                                ClangASTNodeType::Decl);
  }

  bool TraverseStmt(Stmt* s_)
  {
    return TraverseClangASTNode(s_, [&]{ return Base::TraverseStmt(s_); },
                                std::mem_fn(&NodeSearch::tryLocateStmt),
                                ClangASTNodeType::Stmt);
  }

  /**
   * Returns whether or not a node has been found.
   */
  bool hasFoundNode() const
  {
    return static_cast<bool>(_foundNode);
  }

  /**
   * Obtain the information about the found node.
   */
  ASTInfoTree::Node getFoundNode()
  {
    assert(hasFoundNode() && "Not applicable if no node was found.");
    return *_foundNode;
  }

  /**
   * Get the list of children of the found node.
   */
  std::vector<ASTInfoTree::Node> getChildren()
  {
    assert(hasFoundNode() && "Not applicable if no node was found.");

    std::vector<ASTInfoTree::Node> ret;
    std::transform(_children.begin(), _children.end(), std::back_inserter(ret),
                   [](auto entry) {
                     return entry.second;
                   });

    return ret;
  };

protected:
  NodeSearch(const ASTContext& context_)
    : _visitCount(0),
      _context(context_)
  {}

  // Returns whether the passed AST node information matches for the filter.
  virtual bool tryLocateDecl(Decl* d_, size_t index_) = 0;
  virtual bool tryLocateStmt(Stmt* s_, size_t index_) = 0;

  /**
   * The current count where the recursive traversal is at.
   */
  size_t _visitCount;

private:
  template<typename ClangASTNode,
           typename BaseTraverseFn,
           typename LocatorFn>
  bool TraverseClangASTNode(ClangASTNode* n_,
                            BaseTraverseFn traverseInBase_,
                            LocatorFn locatorFn_,
                            ClangASTNodeType typeEnum_)
  {
    if (n_ == nullptr)
      // Do not traverse nodes whom aren't really nodes. Turning on visiting of
      // implicit produce some calls where the pointer is null.
      return true;

    size_t currentVisitedNodeId = ++_visitCount;
    LOG(debug) << "Traverse node #" << currentVisitedNodeId << " @ " << n_;

    if (!_foundNode)
    {
      // If the node has not yet been found, try to find it.
      if (locatorFn_(this, n_, currentVisitedNodeId))
      {
        _foundNode = std::make_unique<ASTInfoTree::Node>();
        _foundNode->_visitId = currentVisitedNodeId;
        _foundNode->_clangMemoryAddress = n_;
        _foundNode->_nodeType = typeEnum_;
      }
      else // TODO: This is debug print, can be removed.
      {
        // If still no success, continue the traversal in the children of the
        // current node.
        LOG(debug) << "Target still not found. Searching in children.";
      }
    }
    else
    {
      if (_traverseStack.top() == _foundNode->_clangMemoryAddress)
      {
        _children[currentVisitedNodeId]._visitId = currentVisitedNodeId;
        _children[currentVisitedNodeId]._clangMemoryAddress = n_;
        _children[currentVisitedNodeId]._nodeType = typeEnum_;
      }
    }

    // We walk the tree recursively via the Traverse* methods until the
    // method that would return from the root of the subtree of the executed
    // traversal is on the top of the stack.
    _traverseStack.push(n_);
    bool b = traverseInBase_();
    _traverseStack.pop();

    return (_foundNode && _foundNode->_visitId == currentVisitedNodeId)
           ? false
           : b;
  }

  const ASTContext& _context;

  /**
   * A stack that records the parents of the currently traversed node up until
   * the root (at the bottom) of the AST subtree on which the traversal is done.
   */
  std::stack<const void*> _traverseStack;

  /**
   * Points to the details of the found AST node once the traversal has
   * actually found it.
   */
  std::unique_ptr<ASTInfoTree::Node> _foundNode;

  /**
   * Contains the direct children's traversal number and information entries
   * for the children of the found node.
   */
  NodeVisitMap _children;
};

/**
 * This class searches for a Clang AST Node based on information available in
 * the database.
 */
class ByDatabaseNodeSearch
  : public NodeSearch
{

public:
  /**
   * Initialise a node visit order labeler class which traverses the AST
   * filtering for the database-backed AST node entry given to the constructor.
   *
   * In this case, the user should call HandleTranslationUnit() with the
   * context_ parameter to search for the node in the parsed AST in-memory.
   */
  ByDatabaseNodeSearch(const ASTContext& context_,
                       model::CppAstNodePtr node_)
    : NodeSearch(context_),
      _locator(context_, std::move(node_))
  {}

  ~ByDatabaseNodeSearch() override = default;

protected:
  bool tryLocateDecl(Decl* d_, size_t /*index_*/) override
  {
    return _locator.matchNodeAgainstLocation(d_);
  }

  bool tryLocateStmt(Stmt* s_, size_t /*index_*/) override
  {
    return _locator.matchNodeAgainstLocation(s_);
  }

private:
  /**
   * The AST node locator which is used to find the Clang AST Node matching
   * the database AST row if the database-row searching constructor was called.
   */
  ASTNodeLocator _locator;

};

/**
 * This class allows for traversing the AST searching for a node that has the
 * given visit number based in visit order.
 */
class ByIdNodeSearch
  : public NodeSearch
{

public:
  /**
   * Initialise a node visit order labeler for the subtree whose root has the
   * visitation ID `nodeId_`.
   *
   * In this case, the traversal should begin with Traverse*() on the node
   * which has the given ID. The traversal will only do one step to the
   * children of the given node.
   */
  ByIdNodeSearch(const ASTContext& context_,
                 size_t nodeId_)
    : NodeSearch(context_),
      _searchNodeId(nodeId_)
  {
    // Minus one is used here, because when the user calls a Traverse*()
    // method after constructing the visitor, it will automatically increase
    // the _visitCount by 1.
    _visitCount = nodeId_ - 1;
  }

  ~ByIdNodeSearch() override = default;

protected:
  bool tryLocateDecl(Decl* /*d_*/, size_t index_) override
  {
    return index_ == _searchNodeId;
  }

  bool tryLocateStmt(Stmt* /*s_*/, size_t index_) override
  {
    return index_ == _searchNodeId;
  }

private:
  /**
   * If the class was instantiated with a node-ID to pick up the traversal
   * from, it is stored here, along with the root of the generated InfoTree.
   */
  const size_t _searchNodeId;
};


template <typename Traverser>
boost::optional<ASTInfoTree::Node>
getNodeInfoFromTraverser(ASTCache* cache_,
                         const cc::service::core::FileId& fileId_,
                         Traverser& traverser_)
{
  if (traverser_.hasFoundNode())
  {
    ASTInfoTree::Node node = traverser_.getFoundNode();
    std::vector<ASTInfoTree::Node> children = traverser_.getChildren();

    if (!children.empty())
    {
      node._hasChildren = true;

      // Cache the visit IDs of the children of this node in the current file
      // so traversal of the children can easily continue.
      std::for_each(children.begin(), children.end(),
                    [cache_, &fileId_](auto child) {
        cache_->storeIdToAddressMapping(fileId_, child._visitId,
                                        child._clangMemoryAddress,
                                        child._nodeType);
      });
    }

    // Add the visit ID of the root node (which was calculated from the top
    // of the tree) to the cache record of the AST.
    cache_->storeIdToAddressMapping(fileId_, node._visitId,
                                    node._clangMemoryAddress,
                                    node._nodeType);

    return node;
  }

  return boost::optional<ASTInfoTree::Node>();
}

} // namespace (anonymous)

namespace cc
{
namespace service
{
namespace language
{

boost::optional<ASTInfoTree::Node>
ASTInfoTree::getNodeDataForAstNode(model::CppAstNodePtr astNode_)
{
  ByDatabaseNodeSearch traverser(_context, std::move(astNode_));
  traverser.HandleTranslationUnit(_context);
  return getNodeInfoFromTraverser(_astCache, _fileId, traverser);
}

boost::optional<ASTInfoTree::Node>
ASTInfoTree::getNodeDataForId(size_t nodeId_)
{
  ByIdNodeSearch traverser(_context, nodeId_);

  // Try the cache first.
  auto clangNode = _astCache->getIdToAddressMapping(_fileId, nodeId_);
  if (!clangNode.first || clangNode.second == ClangASTNodeType::Unknown)
  {
    // Cache miss, try the actual AST.
    traverser.HandleTranslationUnit(_context);
  }
  else
  {
    if (clangNode.second == ClangASTNodeType::Decl)
    {
      Decl* decl = static_cast<Decl*>(clangNode.first);
      traverser.TraverseDecl(decl);
    }
    else if (clangNode.second == ClangASTNodeType::Stmt)
    {
      Stmt* stmt = static_cast<Stmt*>(clangNode.first);
      traverser.TraverseStmt(stmt);
    }
  }

  return getNodeInfoFromTraverser(_astCache, _fileId, traverser);
}

std::vector<ASTInfoTree::Node>
ASTInfoTree::getChildrenForNode(Node node_)
{
  ByIdNodeSearch traverser(_context, node_._visitId);
  if (node_._nodeType == ClangASTNodeType::Decl)
  {
    Decl* decl = static_cast<Decl*>(node_._clangMemoryAddress);
    traverser.TraverseDecl(decl);
  }
  else if (node_._nodeType == ClangASTNodeType::Stmt)
  {
    Stmt* stmt = static_cast<Stmt*>(node_._clangMemoryAddress);
    traverser.TraverseStmt(stmt);
  }

  std::vector<Node> children = traverser.getChildren();

  // Cache the children here too.
  std::for_each(children.begin(), children.end(), [this](auto child) {
    _astCache->storeIdToAddressMapping(_fileId, child._visitId,
                                       child._clangMemoryAddress,
                                       child._nodeType);
  });

  // The "_hasChild" member of the Nodes inside the getChildren() are not filled
  // properly, as only one round is done by the lookup class, so individual
  // lookups are optimised). But for the children information to be proper, an
  // extra lookup has to be made so we properly know if the input node has
  // "grandchildren".
  std::transform(children.begin(), children.end(), children.begin(),
                 [this](Node& node) {
                   return *getNodeDataForId(node._visitId);
                 });

  return children;
}

std::string ASTInfoTree::getNodeType(Node node_)
{
  if (node_._clangMemoryAddress && node_._nodeType != ClangASTNodeType::Unknown)
  {
    if (node_._nodeType == ClangASTNodeType::Decl)
    {
      Decl* decl = static_cast<Decl*>(node_._clangMemoryAddress);
      return std::string(decl->getDeclKindName()) + "Decl";
    }
    else if (node_._nodeType == ClangASTNodeType::Stmt)
    {
      Stmt* stmt = static_cast<Stmt*>(node_._clangMemoryAddress);
      return stmt->getStmtClassName();
    }
  }

  return "<Invalid Node ID.>";
}

std::string ASTInfoTree::dummyGetDetails(Node node_)
{
  std::string str;
  llvm::raw_string_ostream out(str);

  if (node_._clangMemoryAddress && node_._nodeType != ClangASTNodeType::Unknown)
  {
    if (node_._nodeType == ClangASTNodeType::Decl)
    {
      Decl * decl = static_cast<Decl*>(node_._clangMemoryAddress);
      decl->dump(out, /* Deserialize = */ false);
    }
    else if (node_._nodeType == ClangASTNodeType::Stmt)
    {
      Stmt * stmt = static_cast<Stmt*>(node_._clangMemoryAddress);
      stmt->dump(out, _context.getSourceManager());
    }
  }

  out.flush();

  // Split the string so that only the first line is kept.
  str = str.substr(0, str.find('\n'));

  // Also split the first word (the type of the node) because that is sent in
  // a well-typed fashion already over the API.
  if (str.find(" ") != std::string::npos)
    str = str.substr(str.find(" ") + 1, std::string::npos);

  // And cut the memory address of the node too, as it has no value to the user.
  if (str.find(" ") != std::string::npos)
    str = str.substr(str.find(" ") + 1, std::string::npos);

  return util::escapeHtml(str);
}

} //namespace language
} //namespace service
} //namespace cc
