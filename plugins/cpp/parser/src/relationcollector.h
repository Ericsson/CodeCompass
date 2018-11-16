#ifndef CC_PARSER_RELATIONCOLLECTOR_H
#define CC_PARSER_RELATIONCOLLECTOR_H

#include <clang/AST/RecursiveASTVisitor.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppedge.h>
#include <model/cppedge-odb.hxx>

#include <parser/parsercontext.h>

#include <util/logutil.h>

#include <cppparser/filelocutil.h>

namespace cc
{
namespace parser
{

class RelationCollector : public clang::RecursiveASTVisitor<RelationCollector>
{
public:
  RelationCollector(
    ParserContext& ctx_,
    clang::ASTContext& astContext_);

  ~RelationCollector();

  bool VisitFunctionDecl(clang::FunctionDecl* fd_);

  bool VisitValueDecl(clang::ValueDecl* vd_);

  bool VisitCallExpr(clang::CallExpr* ce_);

  static void cleanUp();

private:
  void addEdge(
    const model::FileId& from_,
    const model::FileId& to_,
    model::CppEdge::Type type_,
    model::CppEdgeAttributePtr attr_ = nullptr);

  ParserContext& _ctx;

  static std::unordered_set<model::CppEdgeId> _edgeCache;
  static std::unordered_set<model::CppEdgeAttributeId> _edgeAttrCache;

  std::vector<model::CppEdgePtr> _edges;
  std::vector<model::CppEdgeAttributePtr> _edgeAttributes;

  FileLocUtil _fileLocUtil;
};

} // parser
} // cc

#endif // CC_PARSER_RELATIONCOLLECTOR_H
