#include <service/csharpservice.h>
#include <util/dbutil.h>
#include <util/util.h>
#include <model/file.h>
#include <model/file-odb.hxx>

namespace cc
{
namespace service
{
namespace language
{
typedef odb::query<cc::model::File> FileQuery;
namespace fs = boost::filesystem;
namespace bp = boost::process;
namespace pt = boost::property_tree;

CsharpServiceHandler::CsharpServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
{
  fs::path csharp_path =
   fs::system_complete("../lib/serviceplugin/csharpservice/");

  std::string command("./csharpservice ");
  command.append("'");
  command.append(getDbString());
  command.append("'");
  c = bp::child(bp::start_dir(csharp_path), command);
  try
  {
    csharpQueryHandler.getClientInterface(25000);
  }
  catch (TransportException& ex)
  {
    LOG(error) << "[csharpservice] Starting service failed!";
  }
}

std::string CsharpServiceHandler::getDbString()
{
  pt::ptree _pt;
  pt::read_json(*_datadir + "/project_info.json", _pt);

  return _pt.get<std::string>("database");
}

void CsharpServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  //LOG(info) << "CsharpServiceHandler getFileTypes";
  return_.push_back("CS");
  return_.push_back("Dir");
}

void CsharpServiceHandler::getAstNodeInfo(
        AstNodeInfo& return_,
        const core::AstNodeId& astNodeId_)
{
  //LOG(info) << "csharpQuery.getAstNodeInfo";
  csharpQueryHandler.getAstNodeInfo(return_, astNodeId_);
}

void CsharpServiceHandler::getAstNodeInfoByPosition(
        AstNodeInfo& return_,
        const core::FilePosition& fpos_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fpos_.file));
  });
  csharpQueryHandler.getAstNodeInfoByPosition(return_, file->path, fpos_.pos);
}

void CsharpServiceHandler::getSourceText(
        std::string& return_,
        const core::AstNodeId& astNodeId_)
{
  //LOG(info) << "LOG(info)";
  core::FileRange fileRange;

  csharpQueryHandler.getFileRange(fileRange, astNodeId_);

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
  });
}

void CsharpServiceHandler::getProperties(
        std::map<std::string, std::string>& return_,
        const core::AstNodeId& astNodeId_)
{
  //LOG(info) << "getProperties";
  csharpQueryHandler.getProperties(return_, astNodeId_);
}

void CsharpServiceHandler::getDocumentation(
        std::string& return_,
        const core::AstNodeId& astNodeId_)
{
  //LOG(info) << "getDocumentation";
  csharpQueryHandler.getDocumentation(return_, astNodeId_);
}

void CsharpServiceHandler::getDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_)
{
  //LOG(info) << "getDiagramTypes";
  //csharpQueryHandler.getDiagramTypes(return_, astNodeId_);
}

void CsharpServiceHandler::getDiagram(
        std::string& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t diagramId_)
{
  //LOG(info) << "getDiagram";
  //csharpQueryHandler.getDiagram(return_, astNodeId_, diagramId_);
}

void CsharpServiceHandler::getDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{
  //LOG(info) << "getDiagramLegend";
}

void CsharpServiceHandler::getFileDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_)
{
  //LOG(info) << "getFileDiagramTypes";
}

void CsharpServiceHandler::getFileDiagram(
        std::string& return_,
        const core::FileId& fileId_,
        const int32_t diagramId_)
{
  //LOG(info) << "getFileDiagram";
}

void CsharpServiceHandler::getFileDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{
  //LOG(info) << "getFileDiagramLegend";
}

void CsharpServiceHandler::getReferenceTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_)
{
  LOG(info) << "getReferenceTypes";
  csharpQueryHandler.getReferenceTypes(return_, astNodeId_);
}

std::int32_t CsharpServiceHandler::getReferenceCount(
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_)
{
  LOG(info) << "getReferenceCount";
  return csharpQueryHandler.getReferenceCount(astNodeId_, referenceId_);
}

void CsharpServiceHandler::getReferences(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::vector<std::string>& tags_)
{
  LOG(info) << "getReferences";
  csharpQueryHandler.getReferences(return_, astNodeId_, referenceId_, tags_);
}

void CsharpServiceHandler::getReferencesInFile(
        std::vector<AstNodeInfo>& /* return_ */,
        const core::AstNodeId& /* astNodeId_ */,
        const std::int32_t /* referenceId_ */,
        const core::FileId& /* fileId_ */,
        const std::vector<std::string>& /* tags_ */)
{
  //LOG(info) << "getReferencesInFile";
  // TODO
}

void CsharpServiceHandler::getReferencesPage(
        std::vector<AstNodeInfo>& /* return_ */,
        const core::AstNodeId& /* astNodeId_ */,
        const std::int32_t /* referenceId_ */,
        const std::int32_t /* pageSize_ */,
        const std::int32_t /* pageNo_ */)
{
  //LOG(info) << "getReferencesPage";
  // TODO
}

void CsharpServiceHandler::getFileReferenceTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& /* fileId_*/)
{
  LOG(info) << "getFileReferenceTypes";
  csharpQueryHandler.getFileReferenceTypes(return_);
}

std::int32_t CsharpServiceHandler::getFileReferenceCount(
        const core::FileId& fileId_,
        const std::int32_t referenceId_)
{
  LOG(info) << "getFileReferenceCount";
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fileId_));
  });
  return csharpQueryHandler.getFileReferenceCount(file->path, referenceId_);
}

void CsharpServiceHandler::getFileReferences(
        std::vector<AstNodeInfo>& return_,
        const core::FileId& fileId_,
        const std::int32_t referenceId_)
{
  LOG(info) << "getFileReferences";
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fileId_));
  });
  csharpQueryHandler.getFileReferences(return_, file->path, referenceId_);
}

void CsharpServiceHandler::getSyntaxHighlight(
        std::vector<SyntaxHighlight>& return_,
        const core::FileRange& range_)
{
  //LOG(info) << "getSyntaxHighlight";
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
