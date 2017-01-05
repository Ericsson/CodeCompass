#ifndef LANGUAGESERVICE_H
#define LANGUAGESERVICE_H

#include "language-api/LanguageService.h"
#include "cppservicehelper/cppservicehelper.h"
#include <core-api/common_types.h>

namespace cc
{ 
namespace service
{  
namespace language
{
 
class CppServiceHandler : virtual public LanguageServiceIf
{
public:
  CppServiceHandler(const CppServiceHelper& helper);

  void getAstNodeInfoByPosition(AstNodeInfo& _return,
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters) override;

  void getInfoBox(InfoBox& _return,
    const core::AstNodeId& astNodeId) override;
  
  void getInfoBoxByPosition(InfoBox& _return,
    const core::FilePosition& fpos,
    const std::vector<std::string> & filters) override;

  void getAstNodeInfo(AstNodeInfo& _return,
    const core::AstNodeId& astNodeId) override;

  void getDiagram(std::string& _return,
    const core::AstNodeId& astNodeId,
    const core::DiagramId::type diagramId) override;

  void getLegend(std::string& _return,
    const core::DiagramId::type diagramId) override;

  void getFunctionCallPathDiagram(std::string& _return,
    const core::AstNodeId& astNodeIdFrom,
    const core::AstNodeId& astNodeIdTo) override;

  void getReferences(std::vector<AstNodeInfo> & _return,
    const core::AstNodeId& astNodeId,
    const RefTypes::type referenceId) override;

  void getReferencesInFile(std::vector<AstNodeInfo> & _return,
    const core::AstNodeId& astNodeId,
    const RefTypes::type referenceId,
    const core::FileId& fileId) override;

  void getPage(
    core::RangedHitCountResult& _return,
    const core::AstNodeId& astNodeId,
    const RefTypes::type referenceId,
    const int32_t pageSize,
    const int32_t pageNo) override;

  void getFileDiagram(std::string& _return,
    const core::FileId& fileId, const core::DiagramId::type diagramId) override;

  void getFileReferences(std::vector<AstNodeInfo> & _return,
    const core::FileId& fileId, const RefTypes::type referenceId) override;

  void getTypeDefinitions(
    std::vector<core::AstNodeId>& _return,
    const std::string& path) override;

  void getInfoTree(std::vector<InfoNode> & _return,
    const core::AstNodeId& astNodeId) override;

  void getSubInfoTree(std::vector<InfoNode> & _return,
    const core::AstNodeId& astNodeId, const InfoQuery& query)
      override;

  void getCatalogue(std::vector<InfoNode> & _return) override;

  void getSubCatalogue(std::vector<InfoNode> & _return,
   const InfoQuery& query) override;

  void getInfoTreeForFile(std::vector<InfoNode>&,
    const core::FileId&) override;

  void getSubInfoTreeForFile(std::vector<InfoNode>&,
    const core::FileId&, const InfoQuery&)
      override;

  void getMenuTypes(
      std::vector< core::MenuType> & _return,
      const core::AstNodeId& astNodeId) override;
  
  void getFileMenuTypes(
    std::vector< core::MenuType> & _return,
    const core::FileId& fileId) override;
  
  void getDirMenuTypes(
    std::vector< core::MenuType> & _return,
    const core::FileId& dirId) override;
  
  void getSourceCode(std::string& _return,
      const  core::AstNodeId& astNodeId) override;

  void getDocComment(std::string& _return,
      const  core::AstNodeId& astNodeId) override;
      
  void getBackwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos) override;
  
  void getForwardSlicePos(
    std::vector<core::Range>& _return,
    const core::FilePosition& filePos) override;

  void getSyntaxHighlight(
    std::vector<core::SyntaxHighlight>& _return,
    const core::FileId& fileId) override;

private:
  core::AstNodeId makeAstNodeId( int nodeId_);

  CppServiceHelper helper;
};

} // language
} // service
} // cc

#endif

