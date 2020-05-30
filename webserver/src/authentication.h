#ifndef CC_WEBSERVER_AUTHENTICATION_H
#define CC_WEBSERVER_AUTHENTICATION_H

#include <map>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <util/dynamiclibrary.h>

#include <webserver/authenticator.h>

namespace cc
{
namespace webserver
{

/**
 * Wrapper object that handles authenticating users with loaded authentication
 * backends.
 */
class Authentication
{
public:
  /**
   * Create an authentication engine that lets everyone in.
   */
  Authentication();

  /**
   * Create the authentication engine by parsing the given configuration file.
   */
  Authentication(const std::string& authPluginDir_,
                 boost::filesystem::path authConfigFilePath_);

  bool isEnabled() const;
  const std::string& getAuthPrompt() const { return _authPrompt; }
  std::size_t getSessionLifetime() const { return _sessionLifetime; }

  /**
   * Attempts to authenticate against the authentication backends.
   * If a backend says the given credentials are okay, returns True.
   */
  bool authenticateUsernamePassword(const std::string& username,
                                    const std::string& password) const;

private:
  boost::filesystem::path _authConfigFilePath;
  std::string _authPrompt;
  std::size_t _sessionLifetime;

  std::vector<util::DynamicLibraryPtr> _loadedLibs;
  std::vector<std::unique_ptr<Authenticator>> _authenticators;

  /**
   * Load a webserver authenticator plugin from the given shared object path.
   */
  void createAuthenticator(const std::string& key_,
                           const std::string& path_,
                           const boost::property_tree::ptree& cfgJson_);
};

} // namespace webserver
} // namespace cc

#endif // CC_WEBSERVER_AUTHENTICATION_H
