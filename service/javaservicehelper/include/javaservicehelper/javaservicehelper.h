// $Id$
// Created by Aron Barath, 2013

#ifndef JAVASERVICEHELPER_JAVASERVICEHELPER_H
#define JAVASERVICEHELPER_JAVASERVICEHELPER_H

#include <memory>
#include <stdexcept>

#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>

#include "model/java/javaastnode.h"
#include <model/java/javatype.h>
//#include <model/java/javafunction.h>
//#include <model/java/javavariable.h>

#include "language-api/LanguageService.h"
#include "core-api/result_types.h"

#include "util/odbtransaction.h"

#include "odbquery.h"

namespace cc
{
namespace service
{
namespace language 
{
  
class JavaServiceHelper
{
public:
  JavaServiceHelper(const std::shared_ptr<odb::database>& db_);

  AstNodeInfo getAstNodeInfoByPosition(
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters);

  InfoBox getInfoBox(
    const core::AstNodeId& astNodeId);

  InfoBox getInfoBoxByPosition(
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters);

  void fillInfoBox(InfoBox & ret,
    const model::JavaAstNode & javaAstNode);

  AstNodeInfo getAstNodeInfo(const core::AstNodeId& astNodeId);

  AstNodeInfo getDefinition(const core::AstNodeId& astNodeId);

  std::string getDiagram(const core::AstNodeId& astNodeId,
    const core::DiagramId::type diagramId);

  std::string getLegend(const core::DiagramId::type diagramId);

  std::string getFileDiagram(const core::FileId& fileId,
    const core::DiagramId::type diagramType);
  
  /**
   * This function is implemented especially for function call path diagram
   * between two nodes.
   * @param astNodeId1 from this node
   * @param astNodeId2 to this node
   * @return svg representation of the graph
   */
  std::string getDiagram(const core::AstNodeId& astNodeId1,
    const core::AstNodeId& astNodeId2);
  
  std::vector<AstNodeInfo> getReferences(
    const core::AstNodeId& astNodeId,
    const core::FileId& fileId);

  std::vector<AstNodeInfo> getCallerFunctions(
    const core::AstNodeId& astNodeId,
    const core::FileId& fileId);

  std::vector<AstNodeInfo> getFunctionAssigns(
    const core::AstNodeId& astNodeId,
    const core::FileId& fileId);

  core::RangedHitCountResult getReferencesPage(
    const core::AstNodeId& astNodeId,
    const int pageSize,
    const int pageNo);

  core::RangedHitCountResult getCallerFunctionsPage(
    const core::AstNodeId& astNodeId,
    const int pageSize,
    const int pageNo);

  core::RangedHitCountResult getFunctionAssignsPage(
    const core::AstNodeId& astNodeId,
    const int pageSize,
    const int pageNo);

  std::vector<InfoNode> getInfoTree(
    const core::AstNodeId& astNodeId);

  std::vector<InfoNode> getSubInfoTree(
    const core::AstNodeId& astNodeId, const InfoQuery& infoQuery);

  std::vector<InfoNode> getInfoTreeForFile(const core::FileId&);

  std::vector<InfoNode> getSubInfoTreeForFile(const core::FileId&,
                                              const InfoQuery&);

  std::vector<AstNodeInfo> getFunctionCalls(
    const core::AstNodeId& astNodeId);

  std::shared_ptr<odb::database> getDatabase()
  {
    return db;
  }

  std::string getSourceCode(
    const core::AstNodeId& astNodeId);
  
  std::string getDocComment(
    const core::AstNodeId& astNodeId);

  /**
   * Get the symbolType of a specified AST node
   * @param astNodeId The node id to examine.
   * @return the symbolType of the AST node
   */
  model::SymbolType getNodeKind(const core::AstNodeId& astNodeId);
  
  odb::result<model::JavaAstNode> queryAstNodesByPosition(
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters);

  /**
   * This function is tipically composed with queryAstNodesByPosition.
   * It selects the AST node which is clickable and has the smallest range
   * @param nodes The set we want to select from
   * @return the most appropriate AST node
   */
  model::JavaAstNode selectProperAstNode(odb::result<model::JavaAstNode> nodes);

  std::vector<model::JavaAstNode> unique(
    odb::result<model::JavaAstNode> original);

private:
  enum BuildDiagramRelation
  {
    CONTAINS    = 1,
    CONTAINEDBY = 2,
    PROVIDES    = 4,
    PROVIDEDBY  = 8,
    USES        = 16,
    USEDBY      = 32,
    IN          = CONTAINS | PROVIDES | USES,
    OUT         = CONTAINEDBY | PROVIDEDBY | USEDBY,
    ALL         = IN | OUT
  };
  
  struct BuildDiagramRule
  {
    int level;
    int result;
    BuildDiagramRelation relation;
    
    BuildDiagramRule(int level_, int result_, BuildDiagramRelation relation_)
        : level(level_), result(result_), relation(relation_)
    { }
    
    BuildDiagramRule(int level_, BuildDiagramRelation relation_)
        : BuildDiagramRule(level_, level_ + 1, relation_)
    { }
  };
  
  std::string getClassDiagram(
    const core::AstNodeId& astNodeId);
  
  std::string getFullClassDiagram(
    const core::AstNodeId& astNodeId);
  
  std::string getFunctionCallDiagram(
    const core::AstNodeId& astNodeId);
  
  std::string getIncludeDependencyDiagram(
    const core::FileId& fileId);
  
  std::string getBuildDiagramLocal(
    const core::FileId& fileId);
  
  std::string getBuildDiagramComponent(
    const core::FileId& fileId);
  
  std::string getBuildDiagram(
    const core::FileId& fileId, const std::vector<BuildDiagramRule>& rules);

  core::RangedHitCountResult makeRangedHitCountResult(
    odb::result<model::AstCountGroupByFilesJava> resultSet,
    const int pageSize, const int pageNo);
  
  std::string getDocCommentInTransaction(
    const JavaHashType mangledNameHash);

  std::shared_ptr<odb::database> db;

  util::OdbTransaction transaction;
  JavaOdbQuery query;
};

} // language
} // service
} // cc

#endif // JAVASERVICEHELPER_JAVASERVICEHELPER_H
