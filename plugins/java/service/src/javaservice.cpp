#include <service/javaservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace language
{

JavaServiceHandler::JavaServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _datadir(datadir_),
    _context(context_)
{
  _java_path = pr::search_path("java");
  std::string raw_db_context = getRawDbContext();

  std::vector<std::string> _java_args{
    "-DrawDbContext=" + raw_db_context,
    "-jar",
    "../lib/java/javaservice.jar"
  };
  c = pr::child(_java_path, _java_args, pr::std_out > stdout);

  try {
    javaQueryHandler.getClientInterface(25000);
  } catch (TransportException& ex) {
    LOG(error) << "[javaservice] Starting service failed!";
  }
}

std::string JavaServiceHandler::getRawDbContext() {
  pt::ptree _pt;
  pt::read_json(*_datadir + "/project_info.json", _pt);

  return _pt.get<std::string>("database");
}

void JavaServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("JAVA");
  return_.push_back("Dir");

  LOG(info) << "getFileTypes";
}

void JavaServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getAstNodeInfo";
}

void JavaServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_)
{
  javaQueryHandler.getAstNodeInfoByPosition(return_, fpos_);
  LOG(info) << "getAstNodeInfoByPosition";
}

void JavaServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getSourceText";
}

void JavaServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getProperties";
  javaQueryHandler.getProperties(return_, astNodeId_);
}

void JavaServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getDocumentation";
  javaQueryHandler.getDocumentation(return_, astNodeId_);
}

void JavaServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getDiagramTypes";
}

void JavaServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_)
{
  LOG(info) << "getDiagram";
}

void JavaServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  LOG(info) << "getDiagramLegend";
}

void JavaServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  LOG(info) << "getFileDiagramTypes";
}

void JavaServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_)
{
  LOG(info) << "getFileDiagram";
}

void JavaServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  LOG(info) << "getFileDiagramLegend";
}

void JavaServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_)
{
  javaQueryHandler.getReferenceTypes(return_, astNodeId_);
  LOG(info) << "getReferenceTypes";
}

void JavaServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_)
{
  LOG(info) << "getReferences";
  javaQueryHandler.getReferences(return_, astNodeId_, referenceId_, tags_);
}

std::int32_t JavaServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_)
{
  return javaQueryHandler.getReferenceCount(astNodeId_, referenceId_);
  LOG(info) << "getReferenceCount";
  return 1;
}

void JavaServiceHandler::getReferencesInFile(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const core::FileId& fileId_,
  const std::vector<std::string>& tags_)
{
  LOG(info) << "getReferencesInFile";
}

void JavaServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::int32_t pageSize_,
  const std::int32_t pageNo_)
{
  LOG(info) << "getReferencesPage";
}

void JavaServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  LOG(info) << "getFileReferenceTypes";
}

void JavaServiceHandler::getFileReferences(
  std::vector<AstNodeInfo>& return_,
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{
  LOG(info) << "getFileReferences";
}

std::int32_t JavaServiceHandler::getFileReferenceCount(
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{
  LOG(info) << "getFileReferenceCount";
}

void JavaServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileRange& range_)
{
  LOG(info) << "getSyntaxHighlight";
}

} // language

namespace java
{

// Initialize static member
std::stringstream JavaQueryHandler::thrift_ss;

} // java
} // service
} // cc
