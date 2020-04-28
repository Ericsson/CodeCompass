#ifndef CC_WEBSERVER_SESSION_H
#define CC_WEBSERVER_SESSION_H

#include <chrono>
#include <string>
#include <vector>

namespace cc
{

namespace service
{
namespace authentication
{
class AuthenticationServiceHandler;
}
} // namespace service

namespace webserver
{

class SessionManager;
class MainRequestHandler;

using SessionTimePoint = std::chrono::steady_clock::time_point;

class Session
{
public:
  Session(std::string sessId_, std::string username_);

  SessionTimePoint lastHit() const { return _lastHit; }

  const std::string sessId;
  const std::string username;

private:
  /**
   * The last time the cookie was found in a request for this session.
   */
  SessionTimePoint _lastHit;
};

/**
 * This class can be instantiated from the SessionManager* given to request
 * handler implementations. It can be used to access the current user's session
 * data.
 *
 * Instances are essentially stateless (apart from wrapping over the manager
 * associated) and can be constructed and destroyed on a whim.
 */
class SessionManagerAccess
{
public:
  SessionManagerAccess(SessionManager* manager_) : _manager(manager_) {}

  /**
   * Executes the given function object in the context of the current session.
   * The function object is given as its only parameter a pointer to the
   * session. It might be null if there is no session associated with the
   * current thread of execution.
   */
  template <typename F> auto accessSession(F func)
  {
    if (!_manager)
      throw std::runtime_error(
        "no session manager associated with trampoline.");
    return func(getCurrentSession());
  }

private:
  SessionManager* _manager;

  /**
   * Identifies the session for the request being handled by the current thread.
   */
  static thread_local Session* requestSessionOfCurrentTread;

  /**
   * Retrieves the session object for the current thread of execution.
   */
  Session* getCurrentSession() const;

  // The request handler is privileged to set the session for the current
  // thread of execution.
  friend class cc::webserver::MainRequestHandler;
  static void setCurrentSession(Session* sess_);

  // Authentication service should access the private methods to "write" the
  // session manager.
  friend class cc::service::authentication::AuthenticationServiceHandler;

  // These methods are the wrapper implementations from AuthenticationService's
  // interface to the true session manager.
  bool auth_isRequiringAuthentication() const;
  const std::string& auth_getAuthPrompt() const;
  std::string auth_sessionCookieName();
  bool auth_isCurrentSessionValid();
  std::string auth_usernameAndPassword(const std::string& username_,
                                       const std::string& password_);
  void auth_logout();
};

static_assert(sizeof(SessionManagerAccess) == sizeof(SessionManager*),
              "Do not introduce state to SessionManagerAccess!");

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_SESSION_H
