#ifndef CC_SERVICE_PYTHON_PYTHONSERVICE_H
#define CC_SERVICE_PYTHON_PYTHONSERVICE_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options/variables_map.hpp>

#include <util/odbtransaction.h>
#include <util/util.h>
#include <webserver/servercontext.h>

#include <LanguageService.h>

#include <odb/database.hxx>
#include <model/pyname.h>
#include <model/pyname-odb.hxx>

namespace cc
{
namespace service
{
namespace language
{

class PythonServiceHandler : virtual public LanguageServiceIf
{
public:
   PythonServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getFileTypes(std::vector<std::string>& return_) override;

  void getAstNodeInfo(
    AstNodeInfo& return_,
    const core::AstNodeId& astNodeId_) override;

  void getAstNodeInfoByPosition(
    AstNodeInfo& return_,
    const core::FilePosition& fpos_) override;

  void getSourceText(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override {};

  void getProperties(
    std::map<std::string, std::string>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagram(
    std::string& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t diagramId_) override;

  void getDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getFileDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override;

  void getFileDiagram(
    std::string& return_,
    const core::FileId& fileId_,
    const int32_t diagramId_) override;

  void getFileDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId) override;

  void getReferences(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::vector<std::string>& tags_) override;

  std::int32_t getReferenceCount(
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_) override;

  void getReferencesInFile(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const core::FileId& fileId_,
    const std::vector<std::string>& tags_) override {};

  void getReferencesPage(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::int32_t pageSize_,
    const std::int32_t pageNo_) override {};

  void getFileReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override {};

  void getFileReferences(
    std::vector<AstNodeInfo>& return_,
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override {};

  std::int32_t getFileReferenceCount(
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override {};

  void getSyntaxHighlight(
    std::vector<SyntaxHighlight>& return_,
    const core::FileRange& range_) override {};

  enum ReferenceType
  {
    DEFINITION,
    USAGE,
    THIS_CALLS,
    CALLER,
    PARAMETER,
    LOCAL_VAR,
    DATA_MEMBER,
    METHOD,
    PARENT,
    PARENT_FUNCTION,
    ANNOTATION,
    BASE_CLASS
  };

  enum DiagramType
  {
    FUNCTION_CALL,
    MODULE_DEPENDENCY,
    FUNCTION_USAGE,
    CLASS_USAGE,
    CLASS_OVERVIEW
  };

  model::PYName queryNodeByID(const std::string& id);
  model::PYName queryNodeByPosition(const core::FilePosition& fpos);
  std::vector<model::PYName> queryReferences(const core::AstNodeId& astNodeId, const std::int32_t referenceId);
  std::vector<model::PYName> queryNodesInFile(const core::FileId& fileId, bool definitions);
  std::vector<model::PYName> queryNodes(const odb::query<model::PYName>& odb_query);
  std::vector<std::uint64_t> transformReferences(const std::vector<model::PYName>& references, const model::PYNameID& id);
  std::string getNodeLineValue(const model::PYName& pyname);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::shared_ptr<std::string> _datadir;
  const cc::webserver::ServerContext& _context;

  void setInfoProperties(AstNodeInfo& info, const model::PYName& pyname);
};

} // language
} // service
} // cc

#endif // CC_SERVICE_PYTHON_PYTHONSERVICE_H
