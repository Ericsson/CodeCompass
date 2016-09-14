#ifndef SERVICE_PYTHONSERVICE_PYTHONSERVICE_H
#define SERVICE_PYTHONSERVICE_PYTHONSERVICE_H

#include <memory>
#include <vector>
#include <string>

#include <odb/database.hxx>

#include "langservicelib/utils.h"
#include "language-api/LanguageService.h"
#include <util/odbtransaction.h>

#include <pythonservice/pythonqueryhelper.h>
#include <pythonservice/treehandler.h>


namespace cc
{ 
namespace service
{  
namespace language
{
namespace python
{

class PythonServiceHandler : virtual public LanguageServiceIf
{
public:
  PythonServiceHandler(std::shared_ptr<odb::database> db_);


//
// Complusory declaration of pure virtual methods provied by LanguageServiceIf:
//

  void getAstNodeInfoByPosition(AstNodeInfo& _return,
    const ::cc::service::core::FilePosition& fpos,
    const std::vector<std::string> & filters) override;

  void getInfoBox(InfoBox& _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getInfoBoxByPosition(InfoBox& _return,
    const ::cc::service::core::FilePosition& fpos,
    const std::vector<std::string> & filters) override;

  void getAstNodeInfo(AstNodeInfo& _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getDiagram(std::string& _return,
    const ::cc::service::core::AstNodeId& astNodeId,
    const ::cc::service::core::DiagramId::type diagramId) override;

  void getLegend(std::string& _return,
    const ::cc::service::core::DiagramId::type diagramId) override;

  void getFunctionCallPathDiagram(std::string& _return,
    const  ::cc::service::core::AstNodeId& astNodeIdFrom,
    const ::cc::service::core::AstNodeId& astNodeIdTo) override;

  void getReferences(std::vector<AstNodeInfo> & _return,
    const ::cc::service::core::AstNodeId& astNodeId,
    const RefTypes::type referenceId) override;

  void getReferencesInFile(std::vector<AstNodeInfo> & _return,
    const ::cc::service::core::AstNodeId& astNodeId,
    const RefTypes::type referenceId,
    const ::cc::service::core::FileId& fileId) override;

  void getPage( ::cc::service::core::RangedHitCountResult& _return,
    const ::cc::service::core::AstNodeId& astNodeId,
    const RefTypes::type referenceId,
    const int32_t pageSize,
    const int32_t pageNo) override;

  void getFileDiagram(std::string& _return,
    const ::cc::service::core::FileId& fileId,
    const ::cc::service::core::DiagramId::type diagramId) override;

  void getFileReferences(std::vector<AstNodeInfo> & _return,
    const ::cc::service::core::FileId& fileId,
    const RefTypes::type referenceId) override;

  void getInfoTree(std::vector<InfoNode> & _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getSubInfoTree(std::vector<InfoNode> & _return,
    const ::cc::service::core::AstNodeId& astNodeId,
    const InfoQuery& query) override;

  void getCatalogue(std::vector<InfoNode> & _return) override;

  void getSubCatalogue(std::vector<InfoNode> & _return,
    const InfoQuery& query) override;

  void getInfoTreeForFile(std::vector<InfoNode> & _return,
    const ::cc::service::core::FileId& fileId) override;

  void getSubInfoTreeForFile(std::vector<InfoNode> & _return,
    const ::cc::service::core::FileId& fileId,
    const InfoQuery& query) override;

  void getMenuTypes(std::vector< ::cc::service::core::MenuType> & _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getFileMenuTypes(std::vector< ::cc::service::core::MenuType> & _return,
    const ::cc::service::core::FileId& fileId) override;

  void getDirMenuTypes(std::vector< ::cc::service::core::MenuType> & _return,
    const ::cc::service::core::FileId& dirId) override;

  void getSourceCode(std::string& _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getDocComment(std::string& _return,
    const ::cc::service::core::AstNodeId& astNodeId) override;

  void getBackwardSlicePos(std::vector< ::cc::service::core::Range> & _return,
    const ::cc::service::core::FilePosition& filePos) override;

  void getForwardSlicePos(std::vector< ::cc::service::core::Range> & _return,
    const ::cc::service::core::FilePosition& filePos) override;

  void getTypeDefinitions(std::vector<::cc::service::core::AstNodeId> & _return,
    const std::string& path) override;

  void getSyntaxHighlight(
    std::vector< ::cc::service::core::SyntaxHighlight> & _return,
    const  ::cc::service::core::FileId& fileId) override;

private:
  void getVariableReferences(
    std::vector<AstNodeInfo> & _return,
    const model::PythonAstNode& astNode_,
    const RefTypes::type referenceId_,
    const model::FileId* const fileId_ = 0);

  void getResolvedVariableReferences(
    std::vector<AstNodeInfo> & _return,
    const model::PythonAstNode& astNode_,
    const RefTypes::type referenceId_,
    const model::FileId* const fileId_ = 0);

  void getVariableReferencesPage(
    ::cc::service::core::RangedHitCountResult& _return,
    const model::PythonAstNode& astNode_,
    const RefTypes::type referenceId_,
    const int32_t pageSize_,
    const int32_t pageNo_);

//
// Member variables:
//

private:
  std::shared_ptr<odb::database> db;
  util::OdbTransaction transaction;
  PythonQueryHelper helper;

}; // PythonServiceHandler

} // python
} // language
} // service
} // cc

#endif // SERVICE_PYTHONSERVICE_PYTHONSERVICE_H
