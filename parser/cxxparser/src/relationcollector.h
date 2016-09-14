#ifndef CXXPARSER_PROVIDERELATIONCOLLECTOR_H
#define	CXXPARSER_PROVIDERELATIONCOLLECTOR_H

#include <memory>
#include <map>
#include <set>
#include <string>

#include <clang/AST/RecursiveASTVisitor.h>

#include <parser/sourcemanager.h>

#include <util/util.h>
#include <util/odbtransaction.h>
#include <util/odbobjectcache.h>

#include "model/cxx/cppastnode.h"
#include "model/cxx/cppproviderelation.h"
#include "model/cxx/cppproviderelation-odb.hxx"
#include "model/diagram/node.h"
#include "model/diagram/node-odb.hxx"
#include "model/diagram/edge.h"
#include "model/diagram/edge-odb.hxx"
#include "model/diagram/edgeattribute.h"
#include "model/diagram/edgeattribute-odb.hxx"
#include "model/workspace.h"

#include "filelocutil.h"
#include "cxxparsesession.h"
#include "parseutil.h"

namespace cc
{
namespace parser
{

class RelationCollector : public clang::RecursiveASTVisitor<RelationCollector>
{
public:
  RelationCollector(
    SourceManager& srcMgr_,
    CxxParseSession& session_,
    CXXParser& cxxParser_)
      : _srcMgr(srcMgr_),
        _clangSrcMgr(session_.astContext.getSourceManager()),
        _cxxParser(cxxParser_)
  {
  }
  
  bool VisitFunctionDecl(clang::FunctionDecl* functionDecl)
  {
    //--- Handle only function declarations ---//
    
    if (functionDecl->isThisDeclarationADefinition())
      return true;
    
    //--- Function declaration and definition ---//
    
    model::FilePtr declFilePtr;
    if (!getFileId(declFilePtr, functionDecl->getLocStart()))
      return true;
    model::FileId declFileId = declFilePtr.object_id();
    
    const clang::FunctionDecl* def;
    if (!functionDecl->isDefined(def))
      return true;
    
    model::FilePtr defFilePtr;
    if (!getFileId(defFilePtr, def->getLocStart()))
      return true;
    model::FileId defFileId = defFilePtr.object_id();
    
    //--- Create provide and implement relations ---//
    
    if (declFileId != defFileId)
    {
      std::shared_ptr<model::Edge> edge
        = addEdge(defFileId, declFileId, model::Edge::PROVIDE);
      
      model::EdgeAttribute attr;
      attr.edge  = edge;
      attr.key   = "provide";
      attr.value = functionDecl->getNameAsString();
      
      _cxxParser.getEdgeAttrCache().getOrInsert(
        std::to_string(attr.edge->id) + attr.key + attr.value, attr);
    }
    
    std::shared_ptr<model::Edge> edge = addEdge(
      defFilePtr->parent.object_id(),
      declFilePtr->parent.object_id(),
      model::Edge::IMPLEMENT);
    
    model::EdgeAttribute attr;
    attr.edge  = edge;
    attr.key   = "implement";
    attr.value = defFilePtr->filename + " -> " + declFilePtr->filename;
    
    _cxxParser.getEdgeAttrCache().getOrInsert(
      std::to_string(attr.edge->id) + attr.key + attr.value, attr);
    
    return true;
  }
  
  bool VisitValueDecl(clang::ValueDecl* valueDecl)
  {
    //--- Find user ---//
    
    model::FilePtr userFilePtr;
    if (!getFileId(userFilePtr, valueDecl->getLocStart()))
      return true;
    model::FileId userFileId = userFilePtr.object_id();
    
    //--- Find used ---//
    
    clang::CXXRecordDecl* recordDecl
      = getStrippedType(valueDecl->getType())->getAsCXXRecordDecl();
    
    model::FilePtr usedFilePtr;
    if (!recordDecl || !getFileId(usedFilePtr, recordDecl->getLocStart()))
      return true;
    model::FileId usedFileId = usedFilePtr.object_id();
    
    //--- Create use relation ---//
    
    if (userFileId != usedFileId)
      std::shared_ptr<model::Edge> edge
        = addEdge(userFileId, usedFileId, model::Edge::USE);
    
    std::shared_ptr<model::Edge> edge = addEdge(
      userFilePtr->parent.object_id(),
      usedFilePtr->parent.object_id(),
      model::Edge::DEPEND);
    
    model::EdgeAttribute attr;
    attr.edge  = edge;
    attr.key   = "depend";
    attr.value = userFilePtr->filename + " -> " + usedFilePtr->filename;
    
    _cxxParser.getEdgeAttrCache().getOrInsert(
      std::to_string(attr.edge->id) + attr.key + attr.value, attr);
    
    return true;
  }
  
  bool VisitCallExpr(clang::CallExpr* callExpr)
  {
    //--- Find user ---//
    
    model::FilePtr userFilePtr;
    if (!getFileId(userFilePtr, callExpr->getLocStart()))
      return true;
    model::FileId userFileId = userFilePtr.object_id();
    
    //--- Find used ---//
    
    const clang::Decl* calleeDecl = callExpr->getCalleeDecl();
    
    model::FilePtr usedFilePtr;
    if (!calleeDecl || !getFileId(usedFilePtr, calleeDecl->getLocStart()))
      return true;
    model::FileId usedFileId = usedFilePtr.object_id();
    
    //--- Create use relation ---//
    
    if (userFileId != usedFileId)
      std::shared_ptr<model::Edge> edge
        = addEdge(userFileId, usedFileId, model::Edge::USE);
    
    std::shared_ptr<model::Edge> edge = addEdge(
      userFilePtr->parent.object_id(),
      usedFilePtr->parent.object_id(),
      model::Edge::DEPEND);
    
    model::EdgeAttribute attr;
    attr.edge  = edge;
    attr.key   = "depend";
    attr.value = userFilePtr->filename + " -> " + usedFilePtr->filename;
    
    _cxxParser.getEdgeAttrCache().getOrInsert(
      std::to_string(attr.edge->id) + attr.key + attr.value, attr);
    
    return true;
  }
  
private:
  std::shared_ptr<model::Edge> addEdge(
    const model::FileId& from, const model::FileId& to, model::Edge::Type type)
  {
    std::string fromStr     = std::to_string(from);
    std::string toStr       = std::to_string(to);
    std::string nodeTypeStr = std::to_string(model::Node::FILE);
    
    //--- Add nodes ---//

    std::shared_ptr<model::Node> fromNode(new model::Node);
    fromNode->domain = model::Node::FILE;
    fromNode->domainId = fromStr;
    
    std::shared_ptr<model::Node> toNode(new model::Node);
    toNode->domain = model::Node::FILE;
    toNode->domainId = toStr;
    
    fromNode.reset(new model::Node(_cxxParser.getNodeCache().getOrInsert(
      fromStr + nodeTypeStr, *fromNode)));
    toNode.reset(new model::Node(_cxxParser.getNodeCache().getOrInsert(
      toStr + nodeTypeStr, *toNode)));
    
    //--- Add edge ---//

    std::shared_ptr<model::Edge> edge(new model::Edge);
    edge->from = fromNode;
    edge->to   = toNode;
    edge->type = type;
    
    edge.reset(new model::Edge(_cxxParser.getEdgeCache().getOrInsert(
      std::to_string(fromNode->id) +
      std::to_string(toNode->id) +
      std::to_string(type), *edge)));
    
    return edge;
  }
  
  bool getFileId(model::FilePtr& filePtr_, const clang::SourceLocation& loc_)
  {
    model::FileLoc fileLoc;
    FileLocUtil flu(_srcMgr, _clangSrcMgr);
    bool ok = flu.setInfo(loc_, fileLoc);
    filePtr_ = fileLoc.file;
    return ok;
  }
  
  SourceManager& _srcMgr;
  const clang::SourceManager& _clangSrcMgr;
  CXXParser& _cxxParser;
};

}
}

#endif
