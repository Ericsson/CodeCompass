#include <webserver/authenticator.h>

#include <memory>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <util/logutil.h>

using namespace cc::webserver;

class PlainAuthenticator : public Authenticator
{
public:
  PlainAuthenticator(std::string backendName_,
                     const boost::property_tree::ptree* config_)
    : Authenticator(std::move(backendName_))
  {
    auto authList = config_->get_child_optional("users");
    if (!authList || authList->empty())
    {
      LOG(warning) << "Authentication backend " << backendName()
                   << " configured without any user allowed. No log-in is"
                      " possible through this backend!";
      return;
    }

    for (const auto& authElement : *authList)
    {
      // Lists are represented as a collection of key-less JSON elements.
      _validAuthStrings.emplace_back(
        authElement.second.get_value<std::string>());
    }

    _successfullyConfigured = true;
  }

  bool supportsUsernamePassword() const override { return true; }

  bool authenticateUsernamePassword(const std::string& username,
                                    const std::string& password) const override
  {
    std::string format{username};
    format.push_back(':');
    format.append(password);

    return std::find(_validAuthStrings.begin(),
                     _validAuthStrings.end(),
                     format) != _validAuthStrings.end();
  }

private:
  std::vector<std::string> _validAuthStrings;
};


extern "C" void
instantiateAuthenticator(std::string backendName_,
                         const boost::property_tree::ptree* config_,
                         std::unique_ptr<Authenticator>& ptr_)
{
  ptr_ = std::make_unique<PlainAuthenticator>(std::move(backendName_), config_);
}
