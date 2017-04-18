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

std::unordered_set<model::CppNodeId> RelationCollector::_nodeCache;
std::unordered_set<model::CppEdgeId> RelationCollector::_edgeCache;
std::unordered_set<model::CppEdgeAttributeId> RelationCollector::_edgeAttrCache;

RelationCollector::RelationCollector(
  ParserContext& ctx_,
  clang::ASTContext& astContext_,
  MangledNameCache& mangledNameCache_,
  std::unordered_map<const void*, model::CppAstNodeId>& clangToAstNodeId_)
  : _ctx(ctx_),
    _mangledNameCache(mangledNameCache_),
    _clangToAstNodeId(clangToAstNodeId_),
    _fileLocUtil(astContext_.getSourceManager())
{
}

RelationCollector::~RelationCollector()
{
  _ctx.srcMgr.persistFiles();

  (util::OdbTransaction(_ctx.db))([this]{
    persistAll(_nodes);
    persistAll(_edges);
    persistAll(_edgeAttributes);
  });
}

bool RelationCollector::VisitFunctionDecl(clang::FunctionDecl* fd_)
{
  //--- Handle only function declarations ---//

  if (fd_->isThisDeclarationADefinition())
    return true;

  //--- Function declaration and definition ---//

  model::FilePtr declFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(fd_->getLocStart()));

  if (!declFile)
    return true;

  const clang::FunctionDecl* def;
  if (!fd_->isDefined(def))
    return true;

  model::FilePtr defFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(def->getLocStart()));

  if (!defFile)
    return true;

  //--- Create provide and implement relations ---//

  if (declFile->id != defFile->id)
  {
    model::CppEdgeAttributePtr attr
      = std::make_shared<model::CppEdgeAttribute>();

    attr->key   = "provide";
    attr->value = fd_->getNameAsString();

    addEdge(defFile->id, declFile->id, model::CppEdge::PROVIDE, attr);
  }

  model::CppEdgeAttributePtr attr = std::make_shared<model::CppEdgeAttribute>();

  attr->key   = "implement";
  attr->value = defFile->filename + " -> " + declFile->filename;

  addEdge(
    defFile->parent.object_id(),
    declFile->parent.object_id(),
    model::CppEdge::IMPLEMENT,
    attr);

  return true;
}

bool RelationCollector::VisitValueDecl(clang::ValueDecl* vd_)
{
  //--- Find user ---//

  model::FilePtr userFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(vd_->getLocStart()));

  if (!userFile)
    return true;

  //--- Find used ---//

  clang::CXXRecordDecl* recordDecl
    = getStrippedType(vd_->getType())->getAsCXXRecordDecl();

  if (!recordDecl)
    return true;

  model::FilePtr usedFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(recordDecl->getLocStart()));

  if (!usedFile)
    return true;

  //--- Create use and depend relations ---//

  if (userFile->id != usedFile->id)
    addEdge(userFile->id, usedFile->id, model::CppEdge::USE);

  model::CppEdgeAttributePtr attr = std::make_shared<model::CppEdgeAttribute>();

  attr->key   = "depend";
  attr->value = userFile->filename + " -> " + usedFile->filename;

  addEdge(
    userFile->parent.object_id(),
    usedFile->parent.object_id(),
    model::CppEdge::DEPEND,
    attr);

  return true;
}

bool RelationCollector::VisitCallExpr(clang::CallExpr* ce_)
{
  //--- Find user ---//

  model::FilePtr userFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(ce_->getLocStart()));

  if (!userFile)
    return true;

  //--- Find used ---//

  const clang::Decl* calleeDecl = ce_->getCalleeDecl();

  if (!calleeDecl)
    return true;

  model::FilePtr usedFile = _ctx.srcMgr.getFile(
    _fileLocUtil.getFilePath(calleeDecl->getLocStart()));

  if (!usedFile)
    return true;

  //--- Create use and depend relations ---//

  if (userFile->id != usedFile->id)
    addEdge(userFile->id, usedFile->id, model::CppEdge::USE);

  model::CppEdgeAttributePtr attr = std::make_shared<model::CppEdgeAttribute>();

  attr->key   = "depend";
  attr->value = userFile->filename + " -> " + usedFile->filename;

  addEdge(
    userFile->parent.object_id(),
    usedFile->parent.object_id(),
    model::CppEdge::DEPEND,
    attr);

  return true;
}

void RelationCollector::cleanUp()
{
  _nodeCache.clear();
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

  std::string fromStr     = std::to_string(from_);
  std::string toStr       = std::to_string(to_);
  std::string nodeTypeStr = std::to_string(model::CppNode::FILE);

  //--- Add nodes ---//

  model::CppNodePtr fromNode = std::make_shared<model::CppNode>();
  fromNode->domain = model::CppNode::FILE;
  fromNode->domainId = fromStr;
  fromNode->id = createIdentifier(*fromNode);

  if (_nodeCache.insert(fromNode->id).second)
    _nodes.push_back(fromNode);

  model::CppNodePtr toNode = std::make_shared<model::CppNode>();
  toNode->domain = model::CppNode::FILE;
  toNode->domainId = toStr;
  toNode->id = createIdentifier(*toNode);

  if (_nodeCache.insert(toNode->id).second)
    _nodes.push_back(toNode);

  //--- Add edge ---//

  model::CppEdgePtr edge = std::make_shared<model::CppEdge>();
  edge->from = fromNode;
  edge->to   = toNode;
  edge->type = type_;
  edge->id   = createIdentifier(*edge);

  if (_edgeCache.insert(edge->id).second)
  {
    _edges.push_back(edge);

    //--- Add edge attribute ---//

    if (attr_)
    {
      attr_->edge = edge;
      attr_->id = model::createIdentifier(*attr_);

      if (_edgeAttrCache.insert(attr_->id).second)
        _edgeAttributes.push_back(attr_);
    }
  }
}

} // parser
} // cc
