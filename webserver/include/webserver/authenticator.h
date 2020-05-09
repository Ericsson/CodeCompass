#ifndef CC_WEBSERVER_AUTHENTICATOR_H
#define CC_WEBSERVER_AUTHENTICATOR_H

#include <memory>

#include <boost/property_tree/ptree_fwd.hpp>

#include <util/logutil.h>

namespace cc
{
namespace webserver
{

/**
 * A single authentication backend is implemented by deriving from this class.
 */
class Authenticator
{
protected:
  std::string _backendName;
  bool _successfullyConfigured;

  Authenticator(std::string backendName_)
    : _backendName(std::move(backendName_)), _successfullyConfigured(false)
  {
  }

public:
  Authenticator() = delete;
  Authenticator(const Authenticator&) = delete;
  virtual ~Authenticator() = default;
  Authenticator& operator=(const Authenticator&) = delete;

  const std::string& backendName() const { return _backendName; }
  bool isSuccessfullyConfigured() const { return _successfullyConfigured; }

  /**
   * Indicates whether the derived implementation supports username and
   * password based authentication.
   */
  virtual bool supportsUsernamePassword() const { return false; }

  /**
   * Attempts to authenticate the given username and password against the
   * implemented backend in the derived class.
   */
  virtual bool
  authenticateUsernamePassword(const std::string& /* username */,
                               const std::string& /* password */) const
  {
    if (!supportsUsernamePassword())
      return false;

    LOG(debug) << "Authenticator backend " << _backendName
               << " says it supports username-password authentication, but did "
                  "not did not implement handling! Refusing authentication.";
    return false;
  }
};

using AuthFactoryFnTy = void (*)(std::string backendName_,
                                 const boost::property_tree::ptree* config_,
                                 std::unique_ptr<Authenticator>& ptr_);

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_AUTHENTICATOR_H
