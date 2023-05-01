#ifndef CC_SERVICE_LSP_LSPSERVICE_H
#define CC_SERVICE_LSP_LSPSERVICE_H

#include <boost/property_tree/ptree.hpp>

#include <lspservice/lsp_types.h>

namespace cc
{
namespace service
{
namespace lsp
{

namespace pt = boost::property_tree;

class LspServiceHandler
{
public:
  virtual ~LspServiceHandler() = default;

  // Standard LSP methods
  void virtual getDefinition(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getDeclaration(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getImplementation(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getReferences(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  // Extended LSP methods
  void virtual getDiagramTypes(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getDiagram(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getModuleDiagram(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getParameters(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getLocalVariables(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getReturnType(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getOverridden(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getOverrider(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getRead(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getWrite(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getMethods(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getFriends(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getEnumConstants(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getExpansion(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getUndefinition(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getThisCalls(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getCallsOfThis(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getCallee(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getCaller(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getVirtualCall(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getFunctionPointerCall(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getType(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getAlias(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getImplements(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getDataMember(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  void virtual getUnderlyingType(
    pt::ptree& responseTree_,
    const pt::ptree& params_);

  // Errors
  void getMethodNotFound(pt::ptree& responseTree_, const std::string& method_);
  void getParseError(pt::ptree& responseTree_, const std::exception& ex_);
  void getInternalError(pt::ptree& responseTree_, const std::exception& ex_);
  void getUnknownError(pt::ptree& responseTree_);

};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_LSPSERVICE_H