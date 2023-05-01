#ifndef CC_WEBSERVER_LSPHANDLER_H
#define CC_WEBSERVER_LSPHANDLER_H

#include <memory>
#include <regex>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "webserver/requesthandler.h"

#include <util/logutil.h>
#include <util/dbutil.h>

namespace cc
{
namespace webserver
{

namespace pt = boost::property_tree;

/**
 * Support methods of the Language Server Protocol.
 */
enum class LspMethod
{
  Unknown = 0,
  Definition,
  Declaration,
  Implementation,
  References,
  DiagramTypes,
  Diagram,
  Parameters,
  LocalVariables,
  ReturnType,
  Overridden,
  Overriders,
  Read,
  Write,
  Methods,
  Friends,
  EnumConstants,
  Expansion,
  Undefinition,
  ThisCalls,
  CallsOfThis,
  Callee,
  Caller,
  VirtualCall,
  FunctionPointerCall,
  Type,
  Alias,
  Implements,
  DataMember,
  UnderlyingType,
};

template <class LspServiceT>
class LspHandler : public RequestHandler
{
public:
  std::string key() const override
  {
    return "LspHandler";
  }

  LspHandler(std::unique_ptr<LspServiceT>&& service_)
    : lspService(std::move(service_))
  {
  }

  LspHandler(LspServiceT *service_) : lspService(service_)
  {
  }

  int beginRequest(struct mg_connection *conn_) override
  {
    try
    {
      std::string request = getContent(conn_);
      LOG(debug) << "[LSP] Request content:\n" << request;

      pt::ptree responseTree;
      responseTree.put("jsonrpc", "2.0");

      try
      {
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
            lspService->getDefinition(responseTree, params);
            break;
          }
          case LspMethod::Declaration:
          {
            lspService->getDeclaration(responseTree, params);
            break;
          }
          case LspMethod::Implementation:
          {
            lspService->getImplementation(responseTree, params);
            break;
          }
          case LspMethod::References:
          {
            lspService->getReferences(responseTree, params);
            break;
          }
          case LspMethod::DiagramTypes:
          {
            lspService->getDiagramTypes(responseTree, params);
            break;
          }
          case LspMethod::Diagram:
          {
            lspService->getDiagram(responseTree, params);
            break;
          }
          case LspMethod::Parameters:
          {
            lspService->getParameters(responseTree, params);
            break;
          }
          case LspMethod::LocalVariables:
          {
            lspService->getLocalVariables(responseTree, params);
            break;
          }
          case LspMethod::ReturnType:
          {
            lspService->getReturnType(responseTree, params);
            break;
          }
          case LspMethod::Overridden:
          {
            lspService->getOverridden(responseTree, params);
            break;
          }
          case LspMethod::Overriders:
          {
            lspService->getOverrider(responseTree, params);
            break;
          }
          case LspMethod::Read:
          {
            lspService->getRead(responseTree, params);
            break;
          }
          case LspMethod::Write:
          {
            lspService->getWrite(responseTree, params);
            break;
          }
          case LspMethod::Methods:
          {
            lspService->getMethods(responseTree, params);
            break;
          }
          case LspMethod::Friends:
          {
            lspService->getFriends(responseTree, params);
            break;
          }
          case LspMethod::EnumConstants:
          {
            lspService->getEnumConstants(responseTree, params);
            break;
          }
          case LspMethod::Expansion:
          {
            lspService->getExpansion(responseTree, params);
            break;
          }
          case LspMethod::Undefinition:
          {
            lspService->getUndefinition(responseTree, params);
            break;
          }
          case LspMethod::ThisCalls:
          {
            lspService->getThisCalls(responseTree, params);
            break;
          }
          case LspMethod::  CallsOfThis:
          {
            lspService->getCallsOfThis(responseTree, params);
            break;
          }
          case LspMethod::Callee:
          {
            lspService->getCallee(responseTree, params);
            break;
          }
          case LspMethod::Caller:
          {
            lspService->getCaller(responseTree, params);
            break;
          }
          case LspMethod::VirtualCall:
          {
            lspService->getVirtualCall(responseTree, params);
            break;
          }
          case LspMethod::FunctionPointerCall:
          {
            lspService->getFunctionPointerCall(responseTree, params);
            break;
          }
          case LspMethod::Type:
          {
            lspService->getType(responseTree, params);
            break;
          }
          case LspMethod::Alias:
          {
            lspService->getAlias(responseTree, params);
            break;
          }
          case LspMethod::Implements:
          {
            lspService->getImplements(responseTree, params);
            break;
          }
          case LspMethod::DataMember:
          {
            lspService->getDataMember(responseTree, params);
            break;
          }
          case LspMethod::UnderlyingType:
          {
            lspService->getUnderlyingType(responseTree, params);
            break;
          }
          default:
          {
            LOG(warning) << "[LSP] Unsupported method: '" << method << "'";
            lspService->getMethodNotFound(responseTree, method);
          }
        }
      }
      catch (const pt::ptree_error& ex)
      {
        LOG(warning) << ex.what();
        lspService->getParseError(responseTree, ex);
      }
      catch (const std::exception& ex)
      {
        LOG(warning) << ex.what();
        lspService->getInternalError(responseTree, ex);
      }
      catch (...)
      {
        LOG(warning) << "Unknown exception has been caught";
        lspService->getUnknownError(responseTree);
      }

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

private:
  inline std::string getContent(mg_connection* conn_)
  {
    return std::string(conn_->content, conn_->content + conn_->content_len);
  }

  LspMethod parseMethod(const std::string& method)
  {
    static std::unordered_map<std::string, LspMethod> methodMap = {
      { "textDocument/definition",          LspMethod::Definition },
      { "textDocument/declaration",         LspMethod::Declaration },
      { "textDocument/implementation",      LspMethod::Implementation },
      { "textDocument/references",          LspMethod::References },
      { "diagram/diagramTypes",             LspMethod::DiagramTypes },
      { "diagram/diagram",                  LspMethod::Diagram},
      { "textDocument/parameters",          LspMethod::Parameters},
      { "textDocument/localVariables",      LspMethod::LocalVariables},
      { "textDocument/returnType",          LspMethod::ReturnType},
      { "textDocument/overridden",          LspMethod::Overridden},
      { "textDocument/overriders",          LspMethod::Overriders},
      { "textDocument/read",                LspMethod::Read},
      { "textDocument/write",               LspMethod::Write},
      { "textDocument/methods",             LspMethod::Methods},
      { "textDocument/friends",             LspMethod::Friends},
      { "textDocument/enumConstants",       LspMethod::EnumConstants},
      { "textDocument/expansion",           LspMethod::Expansion},
      { "textDocument/undefinition",        LspMethod::Undefinition},
      { "textDocument/thisCalls",           LspMethod::ThisCalls},
      { "textDocument/callsOfThis",         LspMethod::CallsOfThis},
      { "textDocument/callee",              LspMethod::Callee},
      { "textDocument/caller",              LspMethod::Caller},
      { "textDocument/virtualCall",         LspMethod::VirtualCall},
      { "textDocument/functionPointerCall", LspMethod::FunctionPointerCall},
      { "textDocument/type",                LspMethod::Type},
      { "textDocument/alias",               LspMethod::Alias},
      { "textDocument/implements",          LspMethod::Implements},
      { "textDocument/dataMember",          LspMethod::DataMember},
      { "textDocument/underlyingType",      LspMethod::UnderlyingType},
    };

    auto it = methodMap.find(method);
    if (it != methodMap.end())
      return it->second;
    return LspMethod::Unknown;
  }

  std::unique_ptr<LspServiceT> lspService;
};

} // webserver
} // cc

#endif
