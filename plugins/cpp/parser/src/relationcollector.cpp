#include <iostream>
#include <clang/AST/ASTContext.h>

#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>

#include "symbolhelper.h"
#include "relationcollector.h"

namespace cc
{
namespace parser
{

std::unordered_set<model::CppEdgeId> RelationCollector::_edgeCache;
std::unordered_set<model::CppEdgeAttributeId> RelationCollector::_edgeAttrCache;
std::mutex RelationCollector::_edgeCacheMutex;

RelationCollector::RelationCollector(
  ParserContext& ctx_,
  clang::ASTContext& astContext_)
  : _ctx(ctx_),
    _fileLocUtil(astContext_.getSourceManager())
{
  // Fill edge cache on first object initialization
  // Note that the caches are static members.
  std::lock_guard<std::mutex> cacheLock(_edgeCacheMutex);
  if (_edgeCache.empty())
  {
    util::OdbTransaction{_ctx.db}([this]
    {
      for (const model::CppEdge &edge : _ctx.db->query<model::CppEdge>())
      {
        _edgeCache.insert(edge.id);
      }
      for (const model::CppEdgeAttribute &edgeAttr : _ctx.db->query<model::CppEdgeAttribute>())
      {
        _edgeAttrCache.insert(edgeAttr.id);
      }
    }); // end of transaction
  }
}

RelationCollector::~RelationCollector()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    util::persistAll(_newEdges, _ctx.db);
    util::persistAll(_newEdgeAttributes, _ctx.db);
  });
}

bool RelationCollector::VisitFunctionDecl(clang::FunctionDecl* fd_)
{
  //--- Handle only function declarations ---//

  if (fd_->isThisDeclarationADefinition())
    return true;

  //--- Function declaration and definition ---//

  model::FilePtr declFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(fd_->getBeginLoc()));

  if (!declFile)
    return true;

  const clang::FunctionDecl* def;
  if (!fd_->isDefined(def))
    return true;

  model::FilePtr defFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(def->getBeginLoc()));

  if (!defFile)
    return true;

  //--- Create provide relations ---//

  if (declFile->id != defFile->id)
  {
    model::CppEdgeAttributePtr attr
      = std::make_shared<model::CppEdgeAttribute>();

    attr->key   = "provide";
    attr->value = fd_->getNameAsString();

    addEdge(defFile->id, declFile->id, model::CppEdge::PROVIDE, attr);
  }

  return true;
}

bool RelationCollector::VisitValueDecl(clang::ValueDecl* vd_)
{
  //--- Find user ---//

  model::FilePtr userFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(vd_->getBeginLoc()));

  if (!userFile)
    return true;

  //--- Find used ---//

  clang::CXXRecordDecl* recordDecl
    = getStrippedType(vd_->getType())->getAsCXXRecordDecl();

  if (!recordDecl)
    return true;

  model::FilePtr usedFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(recordDecl->getBeginLoc()));

  if (!usedFile)
    return true;

  //--- Create use relations ---//

  if (userFile->id != usedFile->id)
    addEdge(userFile->id, usedFile->id, model::CppEdge::USE);

  return true;
}

bool RelationCollector::VisitCallExpr(clang::CallExpr* ce_)
{
  //--- Find user ---//

  model::FilePtr userFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(ce_->getBeginLoc()));

  if (!userFile)
    return true;

  //--- Find used ---//

  const clang::Decl* calleeDecl = ce_->getCalleeDecl();

  if (!calleeDecl)
    return true;

  model::FilePtr usedFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(calleeDecl->getBeginLoc()));

  if (!usedFile)
    return true;

  //--- Create use relations ---//

  if (userFile->id != usedFile->id)
    addEdge(userFile->id, usedFile->id, model::CppEdge::USE);

  return true;
}

void RelationCollector::cleanUp()
{
  _edgeCache.clear();
  _edgeAttrCache.clear();
}

void RelationCollector::addEdge(
  const model::FileId& from_,
  const model::FileId& to_,
  model::CppEdge::Type type_,
  model::CppEdgeAttributePtr attr_)
{
  static std::mutex m;
  std::lock_guard<std::mutex> guard(m);

  //--- Add edge ---//

  model::CppEdgePtr edge = std::make_shared<model::CppEdge>();

  edge->from = std::make_shared<model::File>();
  edge->from->id = from_;
  edge->to = std::make_shared<model::File>();
  edge->to->id = to_;

  edge->type = type_;
  edge->id   = createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _newEdges.push_back(edge);

    //--- Add edge attribute ---//

    if (attr_)
    {
      attr_->edge = edge;
      attr_->id = model::createIdentifier(*attr_);

      if (_edgeAttrCache.insert(attr_->id).second)
        _newEdgeAttributes.push_back(attr_);
    }
  }
}

} // parser
} // cc
