#ifndef CC_WEBSERVER_MONGOOSESERVER_H
#define CC_WEBSERVER_MONGOOSESERVER_H

#include <csignal>
#include <functional>
#include <string>
#include <thread>
#include <type_traits>

#include <webserver/mongoose.h>

namespace cc
{
namespace webserver
{
namespace _detail
{

/**
 * RAII wraps handling of a specified signal to a different handler.
 */
class SignalHandler
{
public:
  using SignalHandlerFn = ::sighandler_t;

  SignalHandler(int signal_, SignalHandlerFn newHandler_);
  ~SignalHandler();

private:
  int _originalSignal;
  SignalHandlerFn _originalHandler;
};

/**
 * Wraps the running Mongoose server's context into a C++ RAII object.
 */
class MongooseContext
{
public:
  MongooseContext() : _mongoose(nullptr) {}
  MongooseContext(void* serverData_);
  MongooseContext(const MongooseContext&) = delete;
  MongooseContext(MongooseContext&& rhs) noexcept;
  MongooseContext& operator=(const MongooseContext&) = delete;
  MongooseContext& operator=(MongooseContext&& rhs) noexcept;
  ~MongooseContext() noexcept;

  struct mg_mgr* get() const { return _mongoose; }

private:
  struct mg_mgr* _mongoose;
};

} // namespace _detail

/**
 * Handles the creation and event dispatch of a Mongoose server.
 */
class MongooseServer
{
public:
  using HTTPServiceRequestHandlerType = std::function<void(HTTPRequest&&)>;

  /**
   * Constructs a Mongoose listener server on the given port, and serving the
   * given directory for static resources.
   */
  MongooseServer(std::string listenPort_,
                 std::string documentRoot_,
                 std::string sslCertPath_,
                 std::string sslKeyPath_);

  /**
   * Set the HTTP request URI paths that should be regarded as a service.
   * Calls to such paths will be dispacthed to the service handler callback.
   *
   * These paths should NOT contain the leading '/' of the URIs!
   */
  void setHTTPServiceURIPaths(const std::vector<std::string>* paths_);

  /**
   * Set the current server's handler for HTTP requests for services to the
   * given callback.
   */
  void setHTTPServiceRequestHandler(HTTPServiceRequestHandlerType handler_);

  /**
   * Start handling of incoming requests and dispatching them to the event
   * handler.
   */
  void loop();

  /**
   * Wake up the I/O thread (the Mongoose server's listener) and handle
   * potential responses pushed into the response sockets.
   */
  void wakeUp() const;

private:
  static void signalHandler(int signal_);
  static volatile int exitCode;
  static_assert(
    std::is_same<decltype(&signalHandler),
                 _detail::SignalHandler::SignalHandlerFn>::value,
    "POSIX-interfacing signal handler function should match signature!");

  /**
   * Callback function for the Mongoose server that handles the parsing of
   * incoming requests.
   */
  static void
  nativeRequestHandler(struct mg_connection* conn_, int ev_, void* data_);
  static_assert(
    std::is_same<decltype(&nativeRequestHandler), ::mg_event_handler_t>::value,
    "Mongoose-interfacing handler function should match signature!");

  const std::string _listenPort;
  const std::string _documentRoot;
  const std::string _sslCertPath;
  const std::string _sslKeyPath;
  _detail::MongooseContext _mongoose;
  std::thread::id _loopThreadId;

  HTTPServiceRequestHandlerType _httpServiceHandler;
  const std::vector<std::string>* _httpServicePaths;
};

} // namespace webserver
} // namespace cc


#endif // CC_WEBSERVER_MONGOOSESERVER_H
