#include <service/csharpservice.h>
#include <util/dbutil.h>

namespace cc
{
namespace service
{
namespace language
{

CsharpServiceHandler::CsharpServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
{
  try
  {
    csharpQueryHandler.getClientInterface(25000);
    LOG(info) << "csirip";
  }
  catch (TransportException& ex)
  {
    LOG(error) << "[csharpservice] Starting service failed!";
  }
}

void CsharpServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("CSharp");
  return_.push_back("Dir");
}

    void CsharpServiceHandler::getAstNodeInfo(
            AstNodeInfo& return_,
            const core::AstNodeId& astNodeId_)
    {
      csharpQueryHandler.getAstNodeInfo(return_, astNodeId_);
    }

    void CsharpServiceHandler::getAstNodeInfoByPosition(
            AstNodeInfo& return_,
            const core::FilePosition& fpos_)
    {
      csharpQueryHandler.getAstNodeInfoByPosition(return_, fpos_);
    }

    void CsharpServiceHandler::getSourceText(
            std::string& return_,
            const core::AstNodeId& astNodeId_)
    {
      core::FileRange fileRange;

      /*csharpQueryHandler.getFileRange(fileRange, astNodeId_);

      return_ = _transaction([&, this](){
          model::FilePtr file = _db->query_one<model::File>(
                  FileQuery::id == std::stoull(fileRange.file));

          if (!file) {
            return std::string();
          }

          return cc::util::textRange(
                  file->content.load()->content,
                  fileRange.range.startpos.line,
                  fileRange.range.startpos.column,
                  fileRange.range.endpos.line,
                  fileRange.range.endpos.column);
      });*/
    }

    void CsharpServiceHandler::getProperties(
            std::map<std::string, std::string>& return_,
            const core::AstNodeId& astNodeId_)
    {
      csharpQueryHandler.getProperties(return_, astNodeId_);
    }

    void CsharpServiceHandler::getDocumentation(
            std::string& return_,
            const core::AstNodeId& astNodeId_)
    {
      csharpQueryHandler.getDocumentation(return_, astNodeId_);
    }

    void CsharpServiceHandler::getDiagramTypes(
            std::map<std::string, std::int32_t>& return_,
            const core::AstNodeId& astNodeId_)
    {
      //csharpQueryHandler.getDiagramTypes(return_, astNodeId_);
    }

    void CsharpServiceHandler::getDiagram(
            std::string& return_,
            const core::AstNodeId& astNodeId_,
            const std::int32_t diagramId_)
    {
      //csharpQueryHandler.getDiagram(return_, astNodeId_, diagramId_);
    }

    void CsharpServiceHandler::getDiagramLegend(
            std::string& return_,
            const std::int32_t diagramId_)
    {
      LOG(info) << "getDiagramLegend";
    }

    void CsharpServiceHandler::getFileDiagramTypes(
            std::map<std::string, std::int32_t>& return_,
            const core::FileId& fileId_)
    {
      LOG(info) << "getFileDiagramTypes";
    }

    void CsharpServiceHandler::getFileDiagram(
            std::string& return_,
            const core::FileId& fileId_,
            const int32_t diagramId_)
    {
      LOG(info) << "getFileDiagram";
    }

    void CsharpServiceHandler::getFileDiagramLegend(
            std::string& return_,
            const std::int32_t diagramId_)
    {
      LOG(info) << "getFileDiagramLegend";
    }

    void CsharpServiceHandler::getReferenceTypes(
            std::map<std::string, std::int32_t>& return_,
            const core::AstNodeId& astNodeId_)
    {
      //csharpQueryHandler.getReferenceTypes(return_, astNodeId_);
    }

    std::int32_t CsharpServiceHandler::getReferenceCount(
            const core::AstNodeId& astNodeId_,
            const std::int32_t referenceId_)
    {
      //return csharpQueryHandler.getReferenceCount(astNodeId_, referenceId_);
    }

    void CsharpServiceHandler::getReferences(
            std::vector<AstNodeInfo>& return_,
            const core::AstNodeId& astNodeId_,
            const std::int32_t referenceId_,
            const std::vector<std::string>& tags_)
    {
      //csharpQueryHandler.getReferences(return_, astNodeId_, referenceId_, tags_);
    }

    void CsharpServiceHandler::getReferencesInFile(
            std::vector<AstNodeInfo>& /* return_ */,
            const core::AstNodeId& /* astNodeId_ */,
            const std::int32_t /* referenceId_ */,
            const core::FileId& /* fileId_ */,
            const std::vector<std::string>& /* tags_ */)
    {
      // TODO
    }

    void CsharpServiceHandler::getReferencesPage(
            std::vector<AstNodeInfo>& /* return_ */,
            const core::AstNodeId& /* astNodeId_ */,
            const std::int32_t /* referenceId_ */,
            const std::int32_t /* pageSize_ */,
            const std::int32_t /* pageNo_ */)
    {
      // TODO
    }

    void CsharpServiceHandler::getFileReferenceTypes(
            std::map<std::string, std::int32_t>& return_,
            const core::FileId& /* fileId_*/)
    {
      //csharpQueryHandler.getFileReferenceTypes(return_);
    }

    std::int32_t CsharpServiceHandler::getFileReferenceCount(
            const core::FileId& fileId_,
            const std::int32_t referenceId_)
    {
      //return csharpQueryHandler.getFileReferenceCount(fileId_, referenceId_);
    }

    void CsharpServiceHandler::getFileReferences(
            std::vector<AstNodeInfo>& return_,
            const core::FileId& fileId_,
            const std::int32_t referenceId_)
    {
      //csharpQueryHandler.getFileReferences(return_, fileId_, referenceId_);
    }

    void CsharpServiceHandler::getSyntaxHighlight(
            std::vector<SyntaxHighlight>& return_,
            const core::FileRange& range_)
    {
      /*
      std::vector<std::string> content;
      _transaction([&, this]() {
          //--- Load the file content and break it into lines ---//
          model::FilePtr file = _db->query_one<model::File>(
            FileQuery::id == std::stoull(range_.file));
          if (!file || !file->content.load())
            return;
          std::istringstream s(file->content->content);
          std::string line;
          while (std::getline(s, line))
            content.push_back(line);
      });
      csharpQueryHandler.getSyntaxHighlight(return_, range_, content);
      */
    }

} // language

namespace csharp
{
std::stringstream CSharpQueryHandler::thrift_ss;
}
} // service
} // cc
