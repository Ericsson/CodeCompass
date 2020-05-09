#include <utility>

#include <util/logutil.h>

#include <webserver/session.h>

#include "sessionmanager.h"

static std::string EmptyStr = "";

namespace cc
{
namespace webserver
{

Session::Session(std::string sessId_, std::string username_)
  : sessId(std::move(sessId_)), username(std::move(username_)),
    _lastHit(std::chrono::steady_clock::now())
{
}

thread_local Session* SessionManagerAccess::requestSessionOfCurrentTread = nullptr;

Session* SessionManagerAccess::getCurrentSession() const {
  return requestSessionOfCurrentTread;
}

void SessionManagerAccess::setCurrentSession(Session* sess_)
{
  requestSessionOfCurrentTread = sess_;
}

bool SessionManagerAccess::auth_isRequiringAuthentication() const
{
  return _manager ? _manager->isRequiringAuthentication() : false;
}

const std::string& SessionManagerAccess::auth_getAuthPrompt() const
{
  return _manager ? _manager->getAuthPrompt() : EmptyStr;
}

std::string SessionManagerAccess::auth_sessionCookieName()
{
  if (!_manager)
    throw std::runtime_error("Accessing SessionManager through null!");
  return _manager->getSessionCookieName();
}

bool SessionManagerAccess::auth_isCurrentSessionValid()
{
  if (!_manager)
    throw std::runtime_error("Accessing SessionManager through null!");

  return accessSession(
    [this](const Session* sess_) { return _manager->isValid(sess_); });
}


std::string
SessionManagerAccess::auth_usernameAndPassword(const std::string& username_,
                                               const std::string& password_)
{
  if (!_manager)
    throw std::runtime_error("Accessing SessionManager through null!");

  Session* sess =
    _manager->authenticateUserWithNameAndPassword(username_, password_);

  return sess ? sess->sessId : EmptyStr;
}

void SessionManagerAccess::auth_logout()
{
  if (!_manager)
    throw std::runtime_error("Accessing SessionManager through null!");

  accessSession([this](Session* sess_) { _manager->destroySessionCookie(sess_); });
}

} // namespace webserver
} // namespace cc
