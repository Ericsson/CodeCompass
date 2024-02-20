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
  _transaction([&]() {
      auto nodes = _db->query<model::PYName>(
        odb::query<model::PYName>::file_id == std::stoull(fpos_.file) &&
        odb::query<model::PYName>::line_start == fpos_.pos.line &&
        odb::query<model::PYName>::column_start <= fpos_.pos.column &&
        odb::query<model::PYName>::column_end >= fpos_.pos.column
      );

      if(!nodes.empty())
      {
        model::PYName pyname = *nodes.begin();
        return_.id = std::to_string(pyname.id);
        return_.astNodeValue = pyname.value;
        return_.entityHash = 68;
      }else{
        LOG(info) << "[PYSERVICE] Node not found! (line = " << fpos_.pos.line << " column = " << fpos_.pos.column << ")";
      }
  });
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
  return_.emplace("Definition", DEFINITION);
  return_.emplace("Usage", USAGE);
  return;
}

void PythonServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  LOG(info) << "astNodeID: " << astNodeId_;
  LOG(info) << "referenceId_: " << referenceId_;

  _transaction([&]() {

    std::uint64_t ref_id = _db->query_value<model::PYName>(odb::query<model::PYName>::id == std::stoull(astNodeId_)).ref_id; 

    odb::result<model::PYName> nodes;
    switch (referenceId_)
    {
      case DEFINITION:
        nodes = _db->query<model::PYName>(odb::query<model::PYName>::ref_id == ref_id && odb::query<model::PYName>::is_definition == true);
        break;
      case USAGE:
        nodes = _db->query<model::PYName>(odb::query<model::PYName>::ref_id == ref_id && odb::query<model::PYName>::is_definition == false);
        break;
    }

    for(const model::PYName& pyname : nodes)
    {
      AstNodeInfo info;
      info.id = pyname.id;
      info.astNodeType = pyname.type;
      info.range.file = std::to_string(pyname.file_id);
      info.range.range.startpos.line = pyname.line_start;
      info.range.range.startpos.column = pyname.column_start;
      info.range.range.endpos.line = pyname.line_end;
      info.range.range.endpos.column = pyname.column_end;
      info.astNodeValue = pyname.value;
      return_.push_back(info);
    }

    return;
  });
  
  return;
}

std::int32_t PythonServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_) 
{
  LOG(info) << "[PYSERVICE] " << __func__;
  LOG(info) << "astNodeID: " << astNodeId_;
  LOG(info) << "referenceId_: " << referenceId_;

  std::int32_t ret = 0;
  _transaction([&]() {

    std::uint64_t ref_id = _db->query_value<model::PYName>(odb::query<model::PYName>::id == std::stoull(astNodeId_)).ref_id; 

    switch (referenceId_)
    {
      case DEFINITION:
        ret = _db->query<model::PYName>(odb::query<model::PYName>::ref_id == ref_id && odb::query<model::PYName>::is_definition == true).size();
        break;
      case USAGE:
        ret = _db->query<model::PYName>(odb::query<model::PYName>::ref_id == ref_id && odb::query<model::PYName>::is_definition == false).size();
        break;
    }
    return;
  });

  return ret;
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
  return_["Definition"] = DEFINITION;
  return_["Usage"] = USAGE;
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
