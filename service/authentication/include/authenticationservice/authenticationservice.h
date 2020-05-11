#ifndef CC_SERVICE_AUTHENTICATION_AUTHENTICATIONSERVICE_H
#define CC_SERVICE_AUTHENTICATION_AUTHENTICATIONSERVICE_H

#include <AuthenticationService.h>

#include <memory>
#include <string>
#include <vector>

#include <webserver/servercontext.h>
#include <webserver/session.h>

namespace cc
{
namespace service
{
namespace authentication
{

class AuthenticationServiceHandler : virtual public AuthenticationServiceIf
{
public:
  AuthenticationServiceHandler(const cc::webserver::ServerContext& ctx_);

  bool isRequiringAuthentication() override;

  bool isCurrentSessionValid() override;

  void getAuthPrompt(std::string& ret_) override;

  void loginUsernamePassword(std::string& ret_,
                             const std::string& username_,
                             const std::string& password_) override;

  void logout(std::string& ret_) override;

  void getLoggedInUser(std::string& ret_) override;

private:
  cc::webserver::SessionManagerAccess _sessionManager;
};

} // namespace authentication
} // namespace service
} // namespace cc

#endif // CC_SERVICE_AUTHENTICATION_AUTHENTICATIONSERVICE_H
