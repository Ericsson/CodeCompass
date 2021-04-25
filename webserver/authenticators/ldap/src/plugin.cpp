#include <webserver/authenticator.h>
#include <util/logutil.h>

#include <boost/property_tree/ptree.hpp>

#include <set>

#include "../ldap-cpp/cldap.h"

using namespace cc::webserver;

class LDAPAuthenticator : public Authenticator
{
public:
  LDAPAuthenticator(std::string backendName_,
                    const boost::property_tree::ptree* config_)
    : Authenticator(std::move(backendName_))
  {
    _host = config_->get_optional<std::string>("host").get_value_or("");
    if (_host.empty())
    {
      LOG(warning) << "The LDAP server URI is not provided.";
      return;
    }

    _bindDn = config_->get_optional<std::string>("bindDn").get_value_or("");
    _bindPw = config_->get_optional<std::string>("bindPw").get_value_or("");
    if (_bindDn.empty() || _bindPw.empty())
    {
      LOG(warning) << "Bind username or password is not provided.";
      return;
    }

    _baseDn = config_->get_optional<std::string>("baseDn").get_value_or("");
    if (_baseDn.empty())
    {
      LOG(warning) << "Base distinguished name is not provided.";
      return;
    }

    _uidAttr = config_->get_optional<std::string>("uidAttr").get_value_or("");
    if (_uidAttr.empty())
    {
      LOG(warning) << "UID attribute is not provided.";
      return;
    }

    _successfullyConfigured = true;
  }

  bool supportsUsernamePassword() const override { return true; }

  bool authenticateUsernamePassword(const std::string& username,
                                    const std::string& password) const override
  {
    Ldap::Server ldap;

    if (!ldap.Connect(_host, true))
    {
      LOG(warning) << "LDAP connection unsuccessful: " << ldap.Message();
      return false;
    }

    if (!ldap.Bind(_bindDn, _bindPw))
    {
      LOG(warning) << "LDAP binding unsuccessful: " << ldap.Message();
      return false;
    }

    std::string filter(_uidAttr + "=" + username);
    filter = escapeFilter(filter);
    Ldap::ListEntries result = ldap.Search(_baseDn, Ldap::ScopeTree, filter);
    if(result.size() != 1 || result.begin()->DN().empty())
    {
      return false;
    }

    if (!ldap.Bind(result.begin()->DN(), password))
    {
      LOG(warning) << "LDAP binding unsuccessful: " << ldap.Message();
      return false;
    }

    ldap.Disconnect();
    return true;
  }

private:
  std::string escapeFilter(std::string filter) const
  {
    const std::set<char> toEscape{'\\', '*', '(', ')', '\0'};
    auto iter = std::remove_if(filter.begin(), filter.end(),
                   [&](char c){ return toEscape.count(c) > 0; });
    return filter.substr(0, iter - filter.begin());
  }

private:
  std::string _host;
  std::string _bindDn;
  std::string _bindPw;
  std::string _baseDn;
  std::string _uidAttr;
};

extern "C" void
instantiateAuthenticator(std::string backendName_,
                         const boost::property_tree::ptree* config_,
                         std::unique_ptr<Authenticator>& ptr_)
{
  ptr_ = std::make_unique<LDAPAuthenticator>(std::move(backendName_), config_);
}

