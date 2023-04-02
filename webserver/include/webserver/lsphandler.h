#ifndef CC_WEBSERVER_LSPHANDLER_H
#define CC_WEBSERVER_LSPHANDLER_H

#include <memory>
#include <regex>
#include <unordered_map>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>

#include "webserver/requesthandler.h"

#include <util/logutil.h>
#include <util/dbutil.h>

namespace cc
{
namespace webserver
{

template <class LspServiceT>
class LspHandler : public RequestHandler
{
public:
  std::string key() const override
  {
    return "LspHandler";
  }

  LspHandler(std::unique_ptr<LspServiceT>&& service_) : lspService(std::move(service_))
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

    std::string response{};
    lspService->getLspResponse(response, request);

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

  std::unique_ptr<LspServiceT> lspService;
};

} // webserver
} // cc

#endif
