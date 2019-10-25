#ifndef CC_SERVICE_LSP_LSPSERVICE_H
#define CC_SERVICE_LSP_LSPSERVICE_H

#include <vector>

#include <odb/database.hxx>

#include <service/cppservice.h>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <lspservice/lsp_types.h>

namespace cc
{ 
namespace service
{
namespace lsp
{

class LspServiceHandler
{
public:
  LspServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  std::vector<Location> definition(
    const TextDocumentPositionParams& params_);

  std::vector<Location> references(
    const ReferenceParams& params_);

  CompletionList fileDiagramTypes(
    const DiagramTypeParams& params_);

  CompletionList nodeDiagramTypes(
    const DiagramTypeParams& params_);

  Diagram fileDiagram(
    const DiagramParams& params_);

  Diagram nodeDiagram(
    const DiagramParams& params_);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const cc::webserver::ServerContext& _context;
  language::CppServiceHandler _cppService;
};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_LSPSERVICE_H
