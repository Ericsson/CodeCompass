#include <sstream>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <util/dynamiclibrary.h>
#include <util/logutil.h>

#include "authentication.h"

#define INVALID_CFG_FILE_PATH "/dev/null"
#define CODECOMPASS_DEFAULT_SESSION_LIFETIME 1440 // One hour.

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;


namespace cc
{
namespace webserver
{

Authentication::Authentication()
  : _authConfigFilePath(INVALID_CFG_FILE_PATH), _sessionLifetime(0)
{
}

Authentication::Authentication(const std::string& authPluginDir_,
                               boost::filesystem::path authConfigFilePath_)
  : _authConfigFilePath(std::move(authConfigFilePath_))
{
  if (!fs::is_regular_file(_authConfigFilePath))
  {
    LOG(fatal) << "The specified authentication config file '"
               << _authConfigFilePath << "' does not exist.";
    throw 1;
  }

  std::stringstream ss;
  std::ifstream ifs{_authConfigFilePath.native()};
  if (ifs.fail())
  {
    LOG(fatal) << "The authentication config file could not be opened.";
    throw 2;
  }

  pt::ptree json;
  pt::read_json(ifs, json);

  // Check if the authentication is enabled.
  auto enabledEntry = json.get_optional<bool>("enabled");
  if (!enabledEntry.get_value_or(false))
  {
    LOG(warning) << "Authentication config file exists, but does not contain "
                    "enabled: true.";
    LOG(info) << "Disabling authentication...";

    _authConfigFilePath = INVALID_CFG_FILE_PATH;
    return;
  }

  _authPrompt = json.get_optional<std::string>("prompt").get_value_or("");

  _sessionLifetime = json.get_optional<std::size_t>("session-lifetime")
                       .get_value_or(CODECOMPASS_DEFAULT_SESSION_LIFETIME);

  for (const auto& jsonPair : json)
  {
    const std::string& key = jsonPair.first;
    if (key == "enabled" || key == "prompt" || key == "session_lifetime")
      continue;

    const pt::ptree& subTree = jsonPair.second;
    auto isEnabled = subTree.get_optional<bool>("enabled");
    if (!isEnabled.get_value_or(false))
    {
      LOG(debug) << "Skipping initialisation of authenticator '" << key
                 << "', it is disabled.";
      continue;
    }

    // Try to load the plugin for the authenticator backend.

    std::string pluginPath =
      fs::path(authPluginDir_).append("lib" + key).native();
    pluginPath += "auth" + util::DynamicLibrary::extension();
    createAuthenticator(key, pluginPath, subTree);
  }

  if (_authenticators.empty())
  {
    LOG(warning) << "No authenticators configured! Users will not be able "
                    "to access. Please fix your configuration!";
  }
  else
  {
    std::ostringstream authsStream;
    authsStream << "The following authentication backends are used: ";
    for (const auto& auths : _authenticators)
    {
      authsStream << auths->backendName() << "    ";
    }
    LOG(info) << authsStream.str();
  }
}

bool Authentication::isEnabled() const
{
  return _authConfigFilePath.native() != INVALID_CFG_FILE_PATH;
}

void Authentication::createAuthenticator(
  const std::string& key_,
  const std::string& path_,
  const boost::property_tree::ptree& cfgJson_)
{
  util::DynamicLibraryPtr dlib;
  try
  {
    dlib = std::make_shared<util::DynamicLibrary>(path_);
  }
  catch (const std::runtime_error& re)
  {
    LOG(warning) << "Couldn't load authenticator binary for " << key_
                 << ". This backend will NOT be used! Reason:\n"
                 << re.what();
    return;
  }

  AuthFactoryFnTy initFun = reinterpret_cast<AuthFactoryFnTy>(
    dlib->getSymbol("instantiateAuthenticator"));
  std::unique_ptr<Authenticator> authenticatorImpl;
  initFun(key_, &cfgJson_, authenticatorImpl);

  if (!authenticatorImpl)
  {
    LOG(error) << "Initialising authenticator " << key_
               << " failed: the implementation didn't load.";
    return;
  }

  if (!authenticatorImpl->isSuccessfullyConfigured())
  {
    LOG(error) << "Initialising authenticator " << key_
               << " failed: backend reported configuration error.";
    return;
  }

  _authenticators.emplace_back(std::move(authenticatorImpl));
  _loadedLibs.emplace_back(std::move(dlib));
}

bool Authentication::authenticateUsernamePassword(
  const std::string& username,
  const std::string& password) const
{
  if (!isEnabled())
    return true;

  for (const auto& authenticatorElem : _authenticators)
  {
    if (!authenticatorElem->supportsUsernamePassword())
      continue;

    LOG(debug) << "Authenticating user " << username
               << " with password against " << authenticatorElem->backendName()
               << "...";
    if (authenticatorElem->authenticateUsernamePassword(username, password))
      return true;
  }

  LOG(debug) << "Invalid credentials - no auth backend reported success.";
  return false;
}

} // namespace webserver
} // namespace cc
