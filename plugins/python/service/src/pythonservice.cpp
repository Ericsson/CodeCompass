#include <service/pythonservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace language
{

PythonServiceHandler::PythonServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> /*datadir_*/,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _context(context_) {}

void PythonServiceHandler::getFileTypes(
  std::vector<std::string>& return_) 
{
  return_.push_back("PY");
  return_.push_back("Dir");
  return;
}

void PythonServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

std::int32_t PythonServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return 0;
}

void PythonServiceHandler::getReferencesInFile(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const core::FileId& fileId_,
  const std::vector<std::string>& tags_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::int32_t pageSize_,
  const std::int32_t pageNo_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

void PythonServiceHandler::getFileReferences(
  std::vector<AstNodeInfo>& return_,
  const core::FileId& fileId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

std::int32_t PythonServiceHandler::getFileReferenceCount(
  const core::FileId& fileId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return 0;
}

void PythonServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileRange& range_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  return;
}

} // language
} // service
} // cc
