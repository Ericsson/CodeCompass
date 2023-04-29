#ifndef CC_SERVICE_LSP_CPPLSPSERVICE_H
#define CC_SERVICE_LSP_CPPLSPSERVICE_H

#include <memory>
#include <vector>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <service/cppservice.h>
#include <lspservice/lspservice.h>

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

  void getDiagramTypes(
    pt::ptree& responseTree_,
    const pt::ptree& params_) override final;

  void getDiagram(
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

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  language::CppServiceHandler _cppService;
};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_CPPLSPSERVICE_H