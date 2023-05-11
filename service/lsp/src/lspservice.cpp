#include "lspservice/lspservice.h"

namespace cc
{
namespace service
{
namespace lsp
{

// Standard LSP methods

void LspServiceHandler::getSignature(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/signature");
}

void LspServiceHandler::getDefinition(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/definition");
}

void LspServiceHandler::getDeclaration(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/declaration");
}

void LspServiceHandler::getImplementation(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/implementation");
}

void LspServiceHandler::getReferences(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/references");
}

// Extended LSP methods

void LspServiceHandler::getDiagramTypes(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/diagramTypes");
}

void LspServiceHandler::getDiagram(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/diagram");
}

void LspServiceHandler::getModuleDiagram(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "directory/diagram");
}

void LspServiceHandler::getParameters(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/parameters");
}

void LspServiceHandler::getLocalVariables(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/localVariables");
}

void LspServiceHandler::getOverridden(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/overridden");
}

void LspServiceHandler::getOverrider(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/overriders");
}

void LspServiceHandler::getRead(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/read");
}

void LspServiceHandler::getWrite(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/write");
}

void LspServiceHandler::getMethods(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/methods");
}

void LspServiceHandler::getFriends(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/friends");
}

void LspServiceHandler::getEnumConstants(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/enumConstants");
}

void LspServiceHandler::getExpansion(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/expansion");
}

void LspServiceHandler::getUndefinition(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/undefinition");
}

void LspServiceHandler::getThisCalls(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/thisCalls");
}

void LspServiceHandler::getCallsOfThis(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/callsOfThis");
}

void LspServiceHandler::getCallee(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/callee");
}

void LspServiceHandler::getCaller(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/caller");
}

void LspServiceHandler::getVirtualCall(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/virtualCall");
}

void LspServiceHandler::getFunctionPointerCall(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/functionPointerCall");
}

void LspServiceHandler::getAlias(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/alias");
}

void LspServiceHandler::getImplements(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/implements");
}

void LspServiceHandler::getDataMember(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/dataMember");
}

void LspServiceHandler::getUnderlyingType(
  pt::ptree& responseTree_,
  const pt::ptree&)
{
  getMethodNotFound(responseTree_, "textDocument/underlyingType");
}

// Errors

void LspServiceHandler::getMethodNotFound(
  pt::ptree& responseTree_,
  const std::string& method_)
{
  ResponseError error;
  error.code = ErrorCode::MethodNotFound;
  error.message = std::string("Unsupported method: ").append(method_);
  responseTree_.put_child("error", error.createNode());
}

void LspServiceHandler::getParseError(
  pt::ptree& responseTree_,
  const std::exception& ex_)
{
  ResponseError error;
  error.code = ErrorCode::ParseError;
  error.message = std::string("JSON RPC parsing error: ").append(ex_.what());
  responseTree_.put_child("error", error.createNode());
}

void LspServiceHandler::getInternalError(
  pt::ptree& responseTree_,
  const std::exception& ex_)
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