#include "lspservice/lspservice.h"

namespace cc
{ 
namespace service
{
namespace lsp
{

void LspServiceHandler::getDefinition(pt::ptree& responseTree_, const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/definition");
}

void LspServiceHandler::getImplementation(pt::ptree& responseTree_, const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/implementation");
}

void LspServiceHandler::getReferences(pt::ptree& responseTree_, const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/references");
}

void LspServiceHandler::getDiagramTypes(pt::ptree& responseTree_, const pt::ptree&)
{
  getMethodNotFound(responseTree_, "diagram/diagramTypes");
}

void LspServiceHandler::getDiagram(pt::ptree& responseTree_, const pt::ptree&)
{
  getMethodNotFound(responseTree_, "diagram/diagram");
}

void LspServiceHandler::getMethodNotFound(pt::ptree& responseTree_, const std::string& method_)
{
  ResponseError error;
  error.code = ErrorCode::MethodNotFound;
  error.message = std::string("Unsupported method: ").append(method_);
  responseTree_.put_child("error", error.createNode());
}

void LspServiceHandler::getParseError(pt::ptree& responseTree_, const std::exception& ex_)
{
  ResponseError error;
  error.code = ErrorCode::ParseError;
  error.message = std::string("JSON RPC parsing error: ").append(ex_.what());
  responseTree_.put_child("error", error.createNode());
}

void LspServiceHandler::getInternalError(pt::ptree& responseTree_, const std::exception& ex_)
{
  ResponseError error;
  error.code = ErrorCode::InternalError;
  error.message = ex_.what();
  responseTree_.put_child("error", error.createNode());
}

void LspServiceHandler::getUnknownError(pt::ptree& responseTree_)
{
  ResponseError error;
  error.code = ErrorCode::UnknownError;
  responseTree_.put_child("error", error.createNode());
}

} // lsp
} // service
} // cc