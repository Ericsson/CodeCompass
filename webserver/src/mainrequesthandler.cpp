// Ensure that Mongoose API is defined the same way in all translation units!
#include <webserver/mongoose.h> // IWYU pragma: keep

#include <util/logutil.h>
#include <util/util.h>

#include "mainrequesthandler.h"
#include "sessionmanager.h"

namespace cc
{
namespace webserver
{

/**
 * Executes the given function within the context of the session provided.
 * Any code called inside the function will be able to access the session
 * associated with the thread of execution the function is running on.
 */
template <typename F>
auto MainRequestHandler::executeWithSessionContext(Session* sess_, F func)
{
  SessionManagerAccess::setCurrentSession(sess_);
  try
  {
    func();
    SessionManagerAccess::setCurrentSession(nullptr);
  } catch (...) {
    SessionManagerAccess::setCurrentSession(nullptr);
    throw;
  }
}

void MainRequestHandler::operator()(HTTPRequest& req_)
{
  Session* sessCookie =
    sessionManager->getSessionCookie(req_.getSpecificHeader("Cookie"));
  LOG(debug) << '(' << util::getCurrentDate() << ") " << req_.address << " ["
             << (sessCookie ? sessCookie->username : "Anonymous") << "] "
             << req_.method << ' ' << req_.uri;

  if (req_.uri == "/AuthenticationService")
    // Handle the authentication service specially - it needs access to the
    // session if it exists, but does NOT require a valid session to access.
    return executeWithSessionContext(
      sessCookie, [this, &req_]() { begin_request_handler(req_); });

  if (!sessionManager->isValid(sessCookie))
  {
    sessionManager->destroySessionCookie(sessCookie);
    mg_http_send_error(
      req_.connection, 403, "Accessing this service requires authentication!");
    return;
  }

  executeWithSessionContext(sessCookie,
                            [this, &req_]() { begin_request_handler(req_); });
}

void MainRequestHandler::begin_request_handler(const HTTPRequest& req_)
{
  // Try to find a handler for the URI without the leading '/'.
  auto handler = pluginHandler.getImplementation(req_.uri.substr(1));
  if (handler)
  {
    try
    {
      std::string response = handler->beginRequest(req_);
      // Send HTTP reply to the client.
      mg_send_head(req_.connection,
                   200,
                   response.size(),
                   "Content-Type: application/x-thrift");
      mg_send(req_.connection, response.data(), response.length());
    }
    catch (...)
    {
      LOG(warning) << "Plugin request handler threw an error";
      mg_http_send_error(req_.connection, 500, "Request handler failed.");
    }
  }
  else
    mg_http_send_error(
      req_.connection,
      501,
      "Tried to handle request but no associated implementation was found.");
}

void MainRequestHandler::updateServedPathsFromPluginHandler()
{
  servedPaths.clear();
  servedPaths.reserve(pluginHandler.getImplementationMap().size());
  for (const auto& it : pluginHandler.getImplementationMap())
    servedPaths.push_back(it.first);
}

} // namespace webserver
} // namespace cc
