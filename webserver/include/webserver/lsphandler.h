#ifndef CC_WEBSERVER_LSPHANDLER_H
#define CC_WEBSERVER_LSPHANDLER_H

#include <regex>
#include <unordered_map>

//#include <lspservice/lspservice.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>

#include "webserver/thrifthandler.h"

#include <util/logutil.h>
#include <util/dbutil.h>

namespace cc
{
namespace webserver
{

template <class Processor>
class LspHandler : public ThriftHandler<Processor>
{
public:
  std::string key() const override
  {
    return "LspHandler";
  }

  template<class Handler>
  LspHandler(Handler handler_)
    : ThriftHandler<Processor>(handler_)
  {
  }

  template<class Handler>
  LspHandler(Handler *handler_)
    : ThriftHandler<Processor>(handler_)
  {
  }

int beginRequest(struct mg_connection *conn_) override
{
  using namespace ::apache::thrift;
  using namespace ::apache::thrift::transport;
  using namespace ::apache::thrift::protocol;

  try
  {
    std::string content = getContent(conn_);
    LOG(debug) << "[LSP] Request content:\n" << content;

    // Place LSP message into thrift message
    boost::replace_all(content, "\"", "\\\"");
    std::string request = "[1,\"getLspResponse\",1,0,{\"1\":{\"str\":\"" + content + "\"}}]";

    std::shared_ptr<TTransport> inputBuffer(
      new TMemoryBuffer((std::uint8_t*)request.c_str(), request.length()));

    std::shared_ptr<TTransport> outputBuffer(new TMemoryBuffer(4096));

    std::shared_ptr<TProtocol> inputProtocol(
      new TJSONProtocol(inputBuffer));
    std::shared_ptr<TProtocol> outputProtocol(
      new TJSONProtocol(outputBuffer));

    typename LspHandler<Processor>::CallContext ctx{conn_, nullptr};
    LspHandler<Processor>::_processor.process(inputProtocol, outputProtocol, &ctx);

    TMemoryBuffer *mBuffer = dynamic_cast<TMemoryBuffer*>(outputBuffer.get());

    std::string response = mBuffer->getBufferAsString();

    // Get LSP response from thrift
    std::regex rgx("\\[1,\"getLspResponse\",2,0,\\{\"0\":\\{\"str\":\"(.*)\"\\}\\}\\]");
    std::smatch match;
    std::regex_search(response, match, rgx);
    response = match[1];

    // Remove whitespaces and escape characters
    rgx = "\\s*|\\\\n|\\\\";
    response = std::regex_replace(response, rgx, "");

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
};

} // webserver
} // cc

#endif
