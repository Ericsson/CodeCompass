#include <service/csharpservice.h>
#include <util/dbutil.h>
#include <util/util.h>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <boost/asio.hpp>

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

int CsharpServiceHandler::_thriftServerPort = getThriftServerPort();

CsharpServiceHandler::CsharpServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_),
      _transaction(db_),
      _datadir(datadir_),
      _context(context_)
{
  _csharpQueryHandler.setThriftServerPort(_thriftServerPort);
  fs::path csharp_path =
   fs::system_complete("../lib/serviceplugin/csharpservice/");

  std::string command("./csharpservice ");
  command.append(getDbString());
  command.append(" ");
  command.append(std::to_string(_thriftServerPort));
  ++_thriftServerPort;
  c = bp::child(bp::start_dir(csharp_path), command);
  try
  {
    _csharpQueryHandler.getClientInterface(25000);
  }
  catch (TransportException& ex)
  {
    LOG(error) << "[csharpservice] Starting service failed!";
  }
}

int CsharpServiceHandler::getThriftServerPort()
{
  boost::asio::io_service service;
  boost::system::error_code ec;

  for (int port = 9091; port <= 65535; ++port){
    boost::asio::ip::tcp::acceptor acceptor(service);
    boost::asio::ip::tcp::endpoint endPoint(boost::asio::ip::tcp::v4(), port);
    acceptor.open(endPoint.protocol(), ec);  
    acceptor.bind(endPoint, ec);
    
    if (!ec) {
      acceptor.close(ec);
      return port; 
    }
  }

  return 9091;
}

std::string CsharpServiceHandler::getDbString()
{
  pt::ptree _pt;
  pt::read_json(*_datadir + "/project_info.json", _pt);

  return _pt.get<std::string>("database");
}

void CsharpServiceHandler::getFileTypes(std::vector<std::string>& return_)
{
  return_.push_back("CS");
  return_.push_back("Dir");
}

void CsharpServiceHandler::getAstNodeInfo(
        AstNodeInfo& return_,
        const core::AstNodeId& astNodeId_)
{
  _csharpQueryHandler.getAstNodeInfo(return_, astNodeId_);
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::path == return_.range.file);
  });
  std::stringstream ss;
  ss << file;
  return_.range.file = ss.str();
}

void CsharpServiceHandler::getAstNodeInfoByPosition(
        AstNodeInfo& return_,
        const core::FilePosition& fpos_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fpos_.file));
  });
  _csharpQueryHandler.getAstNodeInfoByPosition(return_, file->path, fpos_.pos);
}

void CsharpServiceHandler::getSourceText(
        std::string& return_,
        const core::AstNodeId& astNodeId_)
{
  core::FileRange fileRange;

  _csharpQueryHandler.getFileRange(fileRange, astNodeId_);

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
  _csharpQueryHandler.getProperties(return_, astNodeId_);
}

void CsharpServiceHandler::getDocumentation(
        std::string& return_,
        const core::AstNodeId& astNodeId_)
{
  _csharpQueryHandler.getDocumentation(return_, astNodeId_);
}

void CsharpServiceHandler::getDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_)
{
  //TODO
  //csharpQueryHandler.getDiagramTypes(return_, astNodeId_);
}

void CsharpServiceHandler::getDiagram(
        std::string& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t diagramId_)
{
  //TODO
  //csharpQueryHandler.getDiagram(return_, astNodeId_, diagramId_);
}

void CsharpServiceHandler::getDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{
  //TODO
}

void CsharpServiceHandler::getFileDiagramTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::FileId& fileId_)
{
  //TODO
}

void CsharpServiceHandler::getFileDiagram(
        std::string& return_,
        const core::FileId& fileId_,
        const int32_t diagramId_)
{
  //TODO
}

void CsharpServiceHandler::getFileDiagramLegend(
        std::string& return_,
        const std::int32_t diagramId_)
{
  //TODO
}

void CsharpServiceHandler::getReferenceTypes(
        std::map<std::string, std::int32_t>& return_,
        const core::AstNodeId& astNodeId_)
{
  _csharpQueryHandler.getReferenceTypes(return_, astNodeId_);
}

std::int32_t CsharpServiceHandler::getReferenceCount(
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_)
{
  return _csharpQueryHandler.getReferenceCount(astNodeId_, referenceId_);
}

void CsharpServiceHandler::getReferences(
        std::vector<AstNodeInfo>& return_,
        const core::AstNodeId& astNodeId_,
        const std::int32_t referenceId_,
        const std::vector<std::string>& tags_)
{
  _csharpQueryHandler.getReferences(return_, astNodeId_, referenceId_, tags_);
  std::vector<AstNodeInfo> ret;
  for (AstNodeInfo nodeinfo : return_)
  {
    model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
    FileQuery::path == nodeinfo.range.file);
    });
    
    std::stringstream ss;
    ss << file->id;
    nodeinfo.range.file = ss.str();
    ret.push_back(nodeinfo);
  }
  return_ = ret;
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
  _csharpQueryHandler.getFileReferenceTypes(return_);
}

std::int32_t CsharpServiceHandler::getFileReferenceCount(
        const core::FileId& fileId_,
        const std::int32_t referenceId_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fileId_));
  });
  return _csharpQueryHandler.getFileReferenceCount(file->path, referenceId_);
}

void CsharpServiceHandler::getFileReferences(
        std::vector<AstNodeInfo>& return_,
        const core::FileId& fileId_,
        const std::int32_t referenceId_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      FileQuery::id == std::stoull(fileId_));
  });
  _csharpQueryHandler.getFileReferences(return_, file->path, referenceId_);
}

void CsharpServiceHandler::getSyntaxHighlight(
        std::vector<SyntaxHighlight>& return_,
        const core::FileRange& range_)
{
  //TODO
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
std::stringstream CSharpQueryHandler::_thriftStream;
int CSharpQueryHandler::_thriftServerPort;
}
} // service
} // cc
