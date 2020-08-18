#ifndef CC_SERVICE_CPPREPARSESERVICE_ASTNODELOCATOR_H
#define CC_SERVICE_CPPREPARSESERVICE_ASTNODELOCATOR_H

#include <clang/AST/ASTContext.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <util/logutil.h>

#include <cppparser/filelocutil.h>

namespace cc
{
namespace service
{
namespace reparse
{

/**
 * This helper class provides a way to match a Clang AST Node (whichever actual
 * type it might have) against a source location stored in the database.
 */
class ASTNodeLocator
{
public:
  ASTNodeLocator(const clang::ASTContext& context_,
                 const model::CppAstNodePtr dbNode_)
    : _context(context_),
      _locUtil(parser::FileLocUtil(context_.getSourceManager())),
      _dbNode(dbNode_)
  {}

  /**
   * Attempts to match the given Clang AST Node type (usually clang::Decl or
   * clang::Stmt) against the CodeCompass C++ AST Node database row that the
   * current class was instantiated with.
   *
   * Returns true if the location of the given node matches the "filter".
   */
  // Template method, because clang::Decl and clang::Stmt are two distinct
  // hierarchies.
  template <typename ClangASTNode>
  bool matchNodeAgainstLocation(ClangASTNode* node_)
  {
    // TODO: What to do with CppAstNodes that are not in a source file? (generated things?)

    if (node_)
    {
      clang::SourceLocation start = node_->getBeginLoc();
      clang::PresumedLoc presumedLoc =
        _context.getSourceManager().getPresumedLoc(start);

      if (presumedLoc.isInvalid())
        return false;

      if (presumedLoc.getLine() != _dbNode->location.range.start.line)
        return false;

      if (presumedLoc.getFilename() != _dbNode->location.file->path)
        return false;

      model::Range declRange;
      _locUtil.setRange(start, node_->getEndLoc(), declRange);

      if (declRange == _dbNode->location.range)
        return true;
    }

    return false;
  }

private:
  const clang::ASTContext& _context;
  parser::FileLocUtil _locUtil;
  model::CppAstNodePtr _dbNode;
};

} // namespace reparse
} // namespace service
} // namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_ASTNODELOCATOR_H
