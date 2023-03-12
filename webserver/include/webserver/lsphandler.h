#ifndef CC_WEBSERVER_LSPHANDLER_H
#define CC_WEBSERVER_LSPHANDLER_H

#include <unordered_map>

#include <lspservice/lspservice.h>

#include "requesthandler.h"

namespace cc
{
namespace webserver
{

class LspHandler : public RequestHandler
{
public:
  LspHandler(const ServerContext& ctx_);

  std::string key() const override
  {
    return "LspHandler";
  }

  int beginRequest(struct mg_connection *conn_) override;

private:
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
  static LspMethod parseMethod(const std::string& method);

  inline std::string getContent(mg_connection* conn_)
  {
    return std::string(conn_->content, conn_->content + conn_->content_len);
  }

  /**
   * A mapping from JSON RPC method (string) to inner representation (LspMethod).
   */
  static std::unordered_map<std::string, LspMethod> _methodMap;

  std::shared_ptr<service::lsp::LspServiceHandler> _service;
};

} // webserver
} // cc

#endif
