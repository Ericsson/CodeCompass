#include <util/logutil.h>
#include <util/util.h>

#include "mainrequesthandler.h"

#include "sessionmanager.h"

static bool isProtected(const char* uri_)
{
  static const std::vector<std::string> UnprotectedFirstDirectories = {
    "doxygen/",    // Resource directory - contains no secret.
    "fonts/",      // Resource directory - contains no secret.
    "images/",     // Resource directory - contains no secret.
    "scripts/",    // Resource directory - contains no secret.
    "style/",      // Resource directory - contains no secret.
    "userguide/",  // Resource directory - contains no secret.
    "favicon.ico", // Resource - contains no secret.
    "login.html",  // Allow to handle web browser auth.
    "ga.txt"       // Google Analytics tracking ID.
  };

  // Swallow the leading '/' of the URI.
  const std::string& uri = uri_ + 1;

  return std::find_if(UnprotectedFirstDirectories.begin(),
                      UnprotectedFirstDirectories.end(),
                      [&uri](const std::string& E) {
                        return uri.find(E) == 0;
                      }) == UnprotectedFirstDirectories.end();
}

static void writeRedirect(struct mg_connection* conn_, const char* loc_)
{
  mg_send_status(conn_, 303); // 303 See Other.
  mg_send_header(conn_, "Location", loc_);
  mg_write(conn_, "\r\n\r\n", 4);
}

namespace cc
{
namespace webserver
{

static void logRequest(const struct mg_connection* conn_, const Session* sess_)
{
  std::string username = sess_ ? sess_->username : "Anonymous";

  LOG(debug) << "(" << util::getCurrentDate() << ") " << conn_->remote_ip << ':'
             << conn_->remote_port << " [" << username << "] " << conn_->uri;
}

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
    auto result = func();
    SessionManagerAccess::setCurrentSession(nullptr);
    return result;
  } catch (...) {
    SessionManagerAccess::setCurrentSession(nullptr);
    throw;
  }
}


int MainRequestHandler::begin_request_handler(struct mg_connection* conn_)
{
  // We advance it by one because of the '/' character.
  const std::string& uri = conn_->uri + 1;

  auto handler = pluginHandler.getImplementation(uri);
  if (handler)
    return handler->beginRequest(conn_);

  if (uri == "ga.txt")
  {
    if (!gaTrackingIdPath.empty())
    {
      mg_send_file(conn_, gaTrackingIdPath.c_str());
      return MG_MORE;
    }
    else
    {
      mg_send_status(conn_, 404); // 404 Not Found.
      mg_send_header(conn_, "Content-Type", "text/plain");
      mg_printf_data(conn_, "%s", "Not Found.");
      return MG_TRUE;
    }
  }

  if (uri.find("doxygen/") == 0)
  {
    mg_send_file(conn_, getDocDirByURI(uri).c_str());
    return MG_MORE;
  }

  // Returning MG_FALSE tells mongoose that we didn't served the request
  // so mongoose should serve it.
  return MG_FALSE;
}

int MainRequestHandler::operator()(struct mg_connection* conn_,
                                   enum mg_event ev_)
{
  if (ev_ == MG_AUTH)
    // Always consider HTTP Digest authentication to be complete. We supply
    // our own authentication system.
    return MG_TRUE;

  if (ev_ != MG_REQUEST)
    // For everything else, bail out.
    return MG_FALSE;

  const char* cookieHeader = mg_get_header(conn_, "Cookie");

  if (strcmp("/AuthenticationService", conn_->uri) == 0)
  {
    Session* sessCookie = sessionManager->getSessionCookie(cookieHeader);
    logRequest(conn_, sessCookie);

    // Handle the authentication service specially - it needs access to the
    // session if it exists, but does NOT require a valid session to access.
    return executeWithSessionContext(
      sessCookie, [this, &conn_]() { return begin_request_handler(conn_); });
  }

  if (!isProtected(conn_->uri))
  {
    // For unprotected endpoints, just serve naturally, without querying the
    // session.
    logRequest(conn_, nullptr);
    return begin_request_handler(conn_);
  }

  Session* sessCookie = sessionManager->getSessionCookie(cookieHeader);
  logRequest(conn_, sessCookie);

  if (!sessionManager->isValid(sessCookie))
  {
    // If authentication is needed and the user does not have a valid session,
    // redirect them to login.
    sessionManager->destroySessionCookie(sessCookie);

    writeRedirect(conn_, "/login.html");
    return MG_TRUE;
  }

  return executeWithSessionContext(
    sessCookie, [this, &conn_]() { return begin_request_handler(conn_); });
}

std::string MainRequestHandler::getDocDirByURI(std::string uri_)
{
  if (uri_.empty())
    return "";

  if (uri_.back() == '/')
    uri_.pop_back();

  std::size_t pos1 = uri_.find('/');
  std::size_t pos2 = uri_.find('/', pos1 + 1);

  std::string ws = uri_.substr(pos1 + 1, pos2 - pos1 - 1);

  std::string file;
  if (pos2 != std::string::npos)
    file = uri_.substr(pos2);

  return dataDir[ws] + "/docs" + file;
}

} // namespace webserver
} // namespace cc
