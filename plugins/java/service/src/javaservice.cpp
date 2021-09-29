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
    _transaction(db_),
    _datadir(datadir_),
    _context(context_)
{
}

void JavaServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("JAVA");
  return_.push_back("Dir");
}

void JavaServiceHandler::getAstNodeInfo(
  AstNodeInfo& return_,
  const core::AstNodeId& astNodeId_)
{

}

void JavaServiceHandler::getAstNodeInfoByPosition(
  AstNodeInfo& return_,
  const core::FilePosition& fpos_)
{

}

void JavaServiceHandler::getSourceText(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{

}

void JavaServiceHandler::getDocumentation(
  std::string& return_,
  const core::AstNodeId& astNodeId_)
{

}

void JavaServiceHandler::getProperties(
  std::map<std::string, std::string>& return_,
  const core::AstNodeId& astNodeId_)
{

}

void JavaServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId_)
{

}

void JavaServiceHandler::getDiagram(
  std::string& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t diagramId_)
{

}

void JavaServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{

}

void JavaServiceHandler::getFileDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{

}

void JavaServiceHandler::getFileDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const int32_t diagramId_)
{

}

void JavaServiceHandler::getFileDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{

}

void JavaServiceHandler::getReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::AstNodeId& astNodeId)
{

}

void JavaServiceHandler::getReferences(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::vector<std::string>& tags_)
{

}

std::int32_t JavaServiceHandler::getReferenceCount(
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_)
{

}

void JavaServiceHandler::getReferencesInFile(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const core::FileId& fileId_,
  const std::vector<std::string>& tags_)
{

}

void JavaServiceHandler::getReferencesPage(
  std::vector<AstNodeInfo>& return_,
  const core::AstNodeId& astNodeId_,
  const std::int32_t referenceId_,
  const std::int32_t pageSize_,
  const std::int32_t pageNo_)
{

}

void JavaServiceHandler::getFileReferenceTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{

}

void JavaServiceHandler::getFileReferences(
  std::vector<AstNodeInfo>& return_,
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{

}

std::int32_t JavaServiceHandler::getFileReferenceCount(
  const core::FileId& fileId_,
  const std::int32_t referenceId_)
{

}

void JavaServiceHandler::getSyntaxHighlight(
  std::vector<SyntaxHighlight>& return_,
  const core::FileRange& range_)
{

}

} // language

namespace java
{

JavaServiceHandler::JavaServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
  _javaProcess.reset(new JavaServiceProcess(*datadir_ + "/java",
                                            context_.compassRoot));
}

void JavaServiceHandler::getJavaString(std::string &str_)
{
  str_ = _config["java-result"].as<std::string>();
}

} // java
} // service
} // cc
