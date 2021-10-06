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

  std::vector<std::string> _java_args{
    // "-DrawDbContext=" + _context.options["database"].as<std::string>(),
    "-jar",
    "../lib/java/javaservice.jar"
  };
  c = pr::child(_java_path, _java_args, pr::std_out > stdout);
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  javaQueryHandler.getClientInterface();

  LOG(info) << "Waiting java service server done.";
}

void JavaServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  LOG(info) << "getFileTypes";

  return_.push_back("JAVA");
  return_.push_back("Dir");
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
  // std::string _return;
  // javaQueryHandler.getJavaString(_return);
  // LOG(info) << _return;
  LOG(info) << "getAstNodeInfoByPosition";
}

void JavaServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getSourceText";
}

void JavaServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getDocumentation";
}

void JavaServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getProperties";
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
  const core::AstNodeId& astNodeId)
{
  LOG(info) << "getReferenceTypes";
}

void JavaServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_)
{
  LOG(info) << "getReferences";
}

std::int32_t JavaServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_)
{
  LOG(info) << "getReferenceCount";
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



} // java
} // service
} // cc
