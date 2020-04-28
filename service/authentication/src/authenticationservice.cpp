#include <util/logutil.h>

#include <authenticationservice/authenticationservice.h>

using namespace cc::webserver;

namespace cc
{
namespace service
{
namespace authentication
{

AuthenticationServiceHandler::AuthenticationServiceHandler(
  const ServerContext& ctx_)
  : _sessionManager(ctx_.sessionManager)
{
}

bool AuthenticationServiceHandler::isRequiringAuthentication()
{
  return _sessionManager.auth_isRequiringAuthentication();
}

bool AuthenticationServiceHandler::isCurrentSessionValid()
{
  return _sessionManager.auth_isCurrentSessionValid();
}

void AuthenticationServiceHandler::getAuthPrompt(std::string& ret_)
{
  ret_ = _sessionManager.auth_getAuthPrompt();
}

void AuthenticationServiceHandler::loginUsernamePassword(
  std::string& ret_,
  const std::string& username_,
  const std::string& password_)
{
  std::string cookieId =
    _sessionManager.auth_usernameAndPassword(username_, password_);
  if (cookieId.empty())
  {
    LOG(info) << "Authentication of " << username_ << " failed.";
    throw IncorrectCredentials{};
  }

  ret_ = _sessionManager.auth_sessionCookieName();
  ret_.push_back('=');
  ret_.append(std::move(cookieId));
  return;
}

void AuthenticationServiceHandler::logout(std::string& ret_)
{
  _sessionManager.auth_logout();
  ret_ = _sessionManager.auth_sessionCookieName();
}

void AuthenticationServiceHandler::getLoggedInUser(std::string& ret_)
{
  ret_ = _sessionManager.accessSession(
    [](const Session* sess_) { return sess_->username; });
}

} // namespace authentication
} // namespace service
} // namespace cc
