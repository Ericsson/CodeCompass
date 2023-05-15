#ifndef CC_SERVICE_LSP_CPPLSPSERVICE_H
#define CC_SERVICE_LSP_CPPLSPSERVICE_H

#include <memory>
#include <vector>

#include <odb/database.hxx>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>
#include <service/cppservice.h>
#include <lspservice/lspservice.h>
#include "projectservice/projectservice.h"
#include "util/graph.h"

namespace cc
{
namespace service
{
namespace lsp
{

class CppLspServiceHandler : public LspServiceHandler
{
public:
  CppLspServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  // Standard LSP methods

  void getDefinition(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getDeclaration(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getImplementation(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getReferences(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  // Extended LSP methods

  void getDiagramTypes(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getDiagram(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getModuleDiagram(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getSignature(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getParameters(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getLocalVariables(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getOverridden(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getOverrider(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getRead(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getWrite(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getMethods(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getFriends(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getEnumConstants(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getExpansion(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getUndefinition(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getThisCalls(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getCallsOfThis(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getCallee(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getCaller(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getVirtualCall(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getFunctionPointerCall(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getAlias(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getImplements(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getDataMember(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getUnderlyingType(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

private:
  void fillResponseTree(pt::ptree& responseTree_,
    const pt::ptree& params_,
    language::CppServiceHandler::ReferenceType refType_,
    bool canBeSingle_ = true);

  std::vector<Location> responseLocations(
    const TextDocumentPositionParams& params_,
    language::CppServiceHandler::ReferenceType refType_);

  CompletionList fileDiagramTypes(
    const DiagramTypeParams& params_);

  CompletionList nodeDiagramTypes(
    const DiagramTypeParams& params_);

  Diagram fileDiagram(
    const DiagramParams& params_);

  Diagram nodeDiagram(
    const DiagramParams& params_);

  void addLocationToIdInDiagram(util::Graph& graph_, const std::string& root_);
  void addPathToIdInFileDiagram(util::Graph& graph_, const std::string& root_);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  language::CppServiceHandler _cppService;
  core::ProjectServiceHandler _projectHandler;
};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_CPPLSPSERVICE_H
