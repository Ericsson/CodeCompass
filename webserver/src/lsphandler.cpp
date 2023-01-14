#include <sstream>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <webserver/lsphandler.h>
#include <webserver/mongoose.h>

#include <util/logutil.h>
#include <util/dbutil.h>

namespace cc
{
namespace webserver
{

using namespace service::lsp;

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

std::unordered_map<std::string, LspHandler::LspMethod> LspHandler::_methodMap = {
  { "textDocument/definition",  LspHandler::LspMethod::Definition },
  { "textDocument/references",  LspHandler::LspMethod::References },
  { "diagram/diagramTypes",     LspHandler::LspMethod::DiagramTypes },
  { "diagram/diagram",          LspHandler::LspMethod::Diagram},
};

LspHandler::LspHandler(const ServerContext& ctx_)
{
  for (fs::directory_iterator it(ctx_.options["workspace"].as<std::string>());
       it != fs::directory_iterator();
       ++it)
  {
    std::string project = it->path().filename().native();

    fs::path projectInfo = it->path();
    projectInfo += "/project_info.json";
    pt::ptree root;
    pt::read_json(projectInfo.native(), root);

    std::string dbName = root.get<std::string>("database", "");
    if (dbName.empty())
      dbName = project;

    // LOG(info)<<dbName<<" "<<ctx_.options.count("database")<<" database exists?";
    
    // std::string connStr = util::updateConnectionString(
    //   ctx_.options["database"].as<std::string>(),
    //   "database",
    //   dbName);

    std::shared_ptr <odb::database> db = util::connectDatabase(dbName);

    if (!db)
    {
      LOG(error)
        << "[LSP] Wrong connection string: '" << dbName << "'";

      throw std::runtime_error("Wrong database!");
    }

    auto datadir = std::make_shared<std::string>(fs::canonical(it->path()).native());
    _service = std::make_shared<LspServiceHandler>(db, datadir, ctx_);
  }
}

int LspHandler::beginRequest(struct mg_connection *conn_)
{
  pt::ptree responseTree;
  responseTree.put("jsonrpc", "2.0");

  try
  {
    std::string request = getContent(conn_);
    LOG(debug) << "[LSP] Request content:\n" << request;

    pt::ptree requestTree;
    std::stringstream requestStream(request);
    pt::read_json(requestStream, requestTree);

    std::string requestId = requestTree.get<std::string>("id");
    responseTree.put("id", requestId);

    std::string method = requestTree.get<std::string>("method");
    pt::ptree& params = requestTree.get_child("params");

    switch (parseMethod(method))
    {
      case LspMethod::Definition:
      {
        TextDocumentPositionParams gotoDefParams;
        gotoDefParams.readNode(params);

        std::vector<Location> gotoDefLocations =
          _service->definition(gotoDefParams);

        if (gotoDefLocations.size() == 1)
        {
          responseTree.put_child("result", gotoDefLocations[0].createNode());
        }
        else if (gotoDefLocations.size() > 1)
        {
          pt::ptree resultNode;
          for (const Location &location : gotoDefLocations)
          {
            resultNode.push_back(std::make_pair("", location.createNode()));
          }
          responseTree.put_child("result", resultNode);
        }
        break;
      }
      case LspMethod::References:
      {
        ReferenceParams refParams;
        refParams.readNode(params);

        std::vector<Location> refLocations = _service->references(refParams);

        pt::ptree resultNode;
        for (const Location& location : refLocations)
        {
          resultNode.push_back(std::make_pair("", location.createNode()));
        }
        responseTree.put_child("result", resultNode);
        break;
      }
      case LspMethod::DiagramTypes:
      {
        DiagramTypeParams diagramTypeParams;
        diagramTypeParams.readNode(params);

        CompletionList diagramTypesResult;
        if (!diagramTypeParams.position)
        {
          diagramTypesResult = _service->fileDiagramTypes(diagramTypeParams);
        }
        else
        {
          diagramTypesResult = _service->nodeDiagramTypes(diagramTypeParams);
        }

        responseTree.put_child("result", diagramTypesResult.createNode());
        break;
      }
      case LspMethod::Diagram:
      {
        DiagramParams diagramParams;
        diagramParams.readNode(params);

        Diagram diagramResult;
        if (!diagramParams.position)
        {
          diagramResult = _service->fileDiagram(diagramParams);
        }
        else
        {
          diagramResult = _service->nodeDiagram(diagramParams);
        }

        responseTree.put("result", diagramResult);
        break;
      }
      default:
      {
        LOG(warning) << "[LSP] Unsupported method: '" << method << "'";

        ResponseError error;
        error.code = ErrorCode::MethodNotFound;
        error.message = std::string("Unsupported method: ").append(method);
        responseTree.put_child("error", error.createNode());
      }
    }
  }
  catch (const pt::ptree_error& ex)
  {
    LOG(warning) << ex.what();

    ResponseError error;
    error.code = ErrorCode::ParseError;
    error.message = std::string("JSON RPC parsing error: ").append(ex.what());
    responseTree.put_child("error", error.createNode());
  }
  catch (const std::exception& ex)
  {
    LOG(warning) << ex.what();

    ResponseError error;
    error.code = ErrorCode::InternalError;
    error.message = ex.what();
    responseTree.put_child("error", error.createNode());
  }
  catch (...)
  {
    LOG(warning) << "Unknown exception has been caught";

    ResponseError error;
    error.code = ErrorCode::UnknownError;
    responseTree.put_child("error", error.createNode());
  }

  try
  {
    std::stringstream responseStream;
    pt::write_json(responseStream, responseTree);
    std::string response = responseStream.str();
    LOG(debug) << "[LSP] Response content:\n" << response << std::endl;

    // Send HTTP reply to the client create headers
    mg_send_header(conn_, "Content-Type", "application/json");
    mg_send_header(
      conn_, "Content-Length", std::to_string(response.length()).c_str());

    // Terminate headers
    mg_write(conn_, "\r\n", 2);

    // Send content
    mg_write(conn_, response.c_str(), response.length());
  }
  catch (const std::exception& ex)
  {
    LOG(warning) << ex.what();
  }
  catch (...)
  {
    LOG(warning) << "Unknown exception has been caught";
  }

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return MG_TRUE;
}

LspHandler::LspMethod LspHandler::parseMethod(const std::string& method)
{
  auto it = _methodMap.find(method);
  if (it != _methodMap.end())
    return it->second;
  return LspMethod::Unknown;
}
} // webserver
} // cc
