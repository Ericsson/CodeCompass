#ifndef CC_SERVICE_LSP_LSPSERVICE_H
#define CC_SERVICE_LSP_LSPSERVICE_H

#include <memory>
#include <vector>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include "LspService.h"
#include <service/cppservice.h>
#include <lspservice/lsp_types.h>

namespace cc
{ 
namespace service
{
namespace lsp
{

class LspServiceHandler : virtual public LspServiceIf
{
public:
  LspServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  virtual void getLspResponse(std::string& _return, const std::string& request) override;

  std::vector<Location> definition(
    const TextDocumentPositionParams& params_);

  std::vector<Location> references(
    const ReferenceParams& params_);

  std::vector<Location> implementation(
    const TextDocumentPositionParams& params_);

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

  language::CppServiceHandler _cppService;


  /**
   * Support methods of the Language Server Protocol.
   */
  enum class LspMethod
  {
    Unknown = 0,
    Definition,
    Implementation,
    References,
    DiagramTypes,
    Diagram
  };

  /**
   * Maps a JSON RPC method (string) to inner representation (LspMethod).
   * @param method The method as JSON RPC method string.
   * @return The matching LspMethod value.
   */
  LspMethod parseMethod(const std::string& method);

  /**
   * A mapping from JSON RPC method (string) to inner representation (LspMethod).
   */
  static std::unordered_map<std::string, LspMethod> _methodMap;
};

} // lsp
} // service
} // cc

#endif // CC_SERVICE_LSP_LSPSERVICE_H