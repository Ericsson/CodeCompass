#ifndef CC_PARSER_RELATIONCOLLECTOR_H
#define CC_PARSER_RELATIONCOLLECTOR_H

#include <clang/AST/RecursiveASTVisitor.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppnode.h>
#include <model/cppnode-odb.hxx>
#include <model/cppedge.h>
#include <model/cppedge-odb.hxx>

#include <parser/parsercontext.h>

#include <util/logutil.h>

#include "filelocutil.h"
#include "manglednamecache.h"

namespace cc
{
namespace parser
{

class RelationCollector : public clang::RecursiveASTVisitor<RelationCollector>
{
public:
  RelationCollector(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_,
    std::unordered_set<model::CppNodeId>& nodeCache_,
    std::unordered_set<model::CppEdgeId>& edgeCache_,
    std::unordered_set<model::CppEdgeAttributeId>&  edgeAttrCache_);

  ~RelationCollector();

  bool VisitFunctionDecl(clang::FunctionDecl* fd_);

  bool VisitValueDecl(clang::ValueDecl* vd_);

  bool VisitCallExpr(clang::CallExpr* ce_);

private:
  std::shared_ptr<model::CppEdge> addEdge(
    const model::FileId& from_,
    const model::FileId& to_,
    model::CppEdge::Type type_);

  template <typename Cont>
  void persistAll(Cont& cont_)
  {
    for (typename Cont::value_type& item : cont_)
    {
      try
      {
        _ctx.db->persist(*item);
      }
      catch (const odb::object_already_persistent& ex)
      {
        LOG(warning)
          << item->toString() << std::endl
          << ex.what() << std::endl
          << "AST nodes in this translation unit will be ignored!";
      }
      catch (const odb::database_exception& ex)
      {
        LOG(debug) << ex.what();
        // TODO: Error code should be checked and rethrow if it is not unique
        // constraint error. Error code may be database specific.
      }
    }
  }

  template <typename K, typename T>
  bool insertToCache(
    std::unordered_set<K>& container_,
    const T& value_)
  {
    static std::mutex cacheMutex;
    std::lock_guard<std::mutex> guard(cacheMutex);

    return container_.insert(value_.id).second;
  }

  ParserContext& _ctx;
  MangledNameCache& _mangledNameCache;
  std::unordered_map<const void*, model::CppAstNodeId>& _clangToAstNodeId;

  std::unordered_set<model::CppNodeId>& _nodeCache;
  std::unordered_set<model::CppEdgeId>& _edgeCache;
  std::unordered_set<model::CppEdgeAttributeId>& _edgeAttrCache;

  std::vector<model::CppNodePtr> _nodes;
  std::vector<model::CppEdgePtr> _edges;
  std::vector<model::CppEdgeAttributePtr> _edgeAttributes;

  FileLocUtil _fileLocUtil;
};

} // parser
} // cc

#endif // CC_PARSER_RELATIONCOLLECTOR_H
