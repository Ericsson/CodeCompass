#include <thread>

#include <util/logutil.h>

#include <webserver/requesthandler.h>

#include "mongooseserver.h"

namespace cc
{
namespace webserver
{

namespace _detail
{

SignalHandler::SignalHandler(int signal_, SignalHandlerFn newHandler_)
  : _originalSignal(signal_)
{
  _originalHandler = signal(signal_, newHandler_);
}

SignalHandler::~SignalHandler()
{
  signal(_originalSignal, _originalHandler);
}

MongooseContext::MongooseContext(void* serverData_) : _mongoose(new mg_mgr())
{
  mg_mgr_init(_mongoose, serverData_);
}

MongooseContext::MongooseContext(MongooseContext&& rhs) noexcept
{
  _mongoose = rhs._mongoose;
  rhs._mongoose = nullptr;
}

MongooseContext& MongooseContext::operator=(MongooseContext&& rhs) noexcept
{
  if (&rhs == this)
    return *this;

  _mongoose = rhs._mongoose;
  rhs._mongoose = nullptr;
  return *this;
}

MongooseContext::~MongooseContext() noexcept
{
  if (_mongoose)
  {
    mg_mgr_free(_mongoose);
    delete _mongoose;
  }
}

} // namespace _detail

volatile int MongooseServer::exitCode = 0;

void MongooseServer::signalHandler(int signal_)
{
  MongooseServer::exitCode = signal_;
}

MongooseServer::MongooseServer(std::string listenPort_,
                               std::string documentRoot_,
                               std::string sslCertPath_,
                               std::string sslKeyPath_)
  : _listenPort(std::move(listenPort_)),
    _documentRoot(std::move(documentRoot_)),
    _sslCertPath(std::move(sslCertPath_)),
    _sslKeyPath(std::move(sslKeyPath_)),
    _mongoose(this)
{
  struct mg_bind_opts opts;
  std::memset(&opts, 0, sizeof(mg_bind_opts));
#if MG_ENABLE_SSL
  if (!_sslCertPath.empty() && !_sslKeyPath.empty())
  {
    opts.ssl_cert = _sslCertPath.c_str();
    opts.ssl_key = _sslKeyPath.c_str();
  }
#endif // MG_ENABLE_SSL

  // Create the listening socket.
  struct mg_connection* listenConnection =
    mg_bind_opt(_mongoose.get(),
                _listenPort.c_str(),
                &MongooseServer::nativeRequestHandler,
                opts);
  mg_set_protocol_http_websocket(listenConnection);
}

void MongooseServer::setHTTPServiceURIPaths(
  const std::vector<std::string>* paths_)
{
  _httpServicePaths = paths_;
}

void MongooseServer::setHTTPServiceRequestHandler(
  MongooseServer::HTTPServiceRequestHandlerType handler_)
{
  _httpServiceHandler = handler_;
}

void MongooseServer::loop()
{
  // Trap termination so the server stops.
  _detail::SignalHandler trapSigint(SIGINT, MongooseServer::signalHandler);
  _detail::SignalHandler trapSigterm(SIGTERM, MongooseServer::signalHandler);

  _loopThreadId = std::this_thread::get_id();

  while (!exitCode)
    mg_mgr_poll(_mongoose.get(), 1000);
}

void MongooseServer::wakeUp() const
{
  if (std::this_thread::get_id() == _loopThreadId)
    throw std::runtime_error(
      "Called MongooseServer::wakeUp() from stack of loop()!");

  mg_broadcast(_mongoose.get(), &nativeRequestHandler, nullptr, 0);
}

void MongooseServer::nativeRequestHandler(struct mg_connection* conn_,
                                          int ev_,
                                          void* data_)
{
  if (ev_ == MG_EV_HTTP_REQUEST)
  {
    auto* nativeRequest = reinterpret_cast<struct http_message*>(data_);
    const auto* handlerInstance =
      reinterpret_cast<MongooseServer*>(conn_->mgr->user_data);

    HTTPRequest handlerRequest;
    handlerRequest.uri =
      std::string{nativeRequest->uri.p, nativeRequest->uri.len};

    // Check if the service is to be dispatched to a service handler
    // implementation.
    bool isServiceCall = false;
    for (const std::string& servicePath : *handlerInstance->_httpServicePaths)
    {
      // Searching from the 1st character as ServicePaths do not contain the
      // leading '/'.
      if (handlerRequest.uri.find(servicePath, 1) == 1)
      {
        isServiceCall = true;
        break;
      }
    }

    if (!isServiceCall)
    {
      // Plain old HTTP request to just download a file resource from the
      // "www-data", handle it as such.
      struct mg_serve_http_opts httpOpts;
      std::memset(&httpOpts, 0, sizeof(struct mg_serve_http_opts));
      httpOpts.document_root = handlerInstance->_documentRoot.c_str();
      mg_serve_http(conn_, nativeRequest, httpOpts);
      return;
    }

    // Otherwise, dispatch the call to the service handler.

    handlerRequest.connection = conn_;
    mg_conn_addr_to_str(conn_,
                        &handlerRequest.address[0],
                        sizeof(handlerRequest.address),
                        MG_SOCK_STRINGIFY_REMOTE | MG_SOCK_STRINGIFY_IP |
                          MG_SOCK_STRINGIFY_PORT);

    std::size_t protoLen =
      std::min(sizeof(handlerRequest.protocol) - 1, nativeRequest->proto.len);
    std::memcpy(&handlerRequest.protocol, nativeRequest->proto.p, protoLen);
    handlerRequest.protocol[protoLen] = '\0';

    std::size_t methodLen =
      std::min(sizeof(handlerRequest.method) - 1, nativeRequest->method.len);
    std::memcpy(&handlerRequest.method, nativeRequest->method.p, methodLen);
    handlerRequest.method[methodLen] = '\0';

    handlerRequest.query = std::string{nativeRequest->query_string.p,
                                       nativeRequest->query_string.len};
    handlerRequest.body =
      std::string{nativeRequest->body.p, nativeRequest->body.len};

    for (std::size_t hdrIndex = 0;
         nativeRequest->header_names[hdrIndex].len > 0;
         ++hdrIndex)
    {
      // While there are valid headers in the request to parse...
      std::string headerName{nativeRequest->header_names[hdrIndex].p,
                             nativeRequest->header_names[hdrIndex].len};
      std::string headerValue{nativeRequest->header_values[hdrIndex].p,
                              nativeRequest->header_values[hdrIndex].len};
      handlerRequest.headers.emplace_back(
        std::make_pair(std::move(headerName), std::move(headerValue)));
    }

    handlerInstance->_httpServiceHandler(std::move(handlerRequest));
    return;
  }

  // IMPORTANT: After this function returns, anything inside data_ should be
  // considered destructed!
}

} // namespace webserver
} // namespace cc

