#ifndef CC_WEBSERVER_SESSIONMANAGER_H
#define CC_WEBSERVER_SESSIONMANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include <webserver/session.h>

namespace cc
{
namespace webserver
{

class Authentication;

/**
 * Keeps track of the active authenticated sessions to the server and handles
 * authenticating users.
 */
class SessionManager
{
public:
  SessionManager(const Authentication* authEngine_);

  bool isRequiringAuthentication() const;

  /**
   * Returns the "authentication prompt" the server was configured with. This
   * is a free-form text the administrator can use to describe their server.
   */
  const std::string& getAuthPrompt() const;

  /**
   * Returns the name of the session cookie that the server searches for.
   */
  std::string getSessionCookieName() const;

  /**
   * Parse the given HTTP header containing all cookies in the request, and
   * fetch the session cookie, if exists.
   */
  Session* getSessionCookie(const char* cookieHeader_);

  /**
   * Deletes the given session cookie from memory.
   */
  void destroySessionCookie(Session* session_);

  bool isValid(const Session* session_) const;

  Session* authenticateUserWithNameAndPassword(const std::string& username_,
                                               const std::string& password_);

private:
  const Authentication* _authEngine;

  mutable std::mutex _sessionMapLock;
  std::unordered_map<std::string, Session> _sessions;

  void cleanupOldSessions();
};

} // namespace webserver
} // namespace cc


#endif // CC_WEBSERVER_SESSIONMANAGER_H
