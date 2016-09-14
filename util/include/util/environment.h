#ifndef __CODECOMPASS_UTIL_ENVIRONMENT_H__
#define __CODECOMPASS_UTIL_ENVIRONMENT_H__

#include <string>

namespace cc
{
namespace util
{

/**
 * Environment management class.
 */
class Environment
{
public:
  /**
   * Inits the environmnet. This must be called before anything else on this
   * class.
   */
  static void init();

  /**
   * Returns the installation directory of the CodeCompass.
   */
  static const std::string& getInstallPath();

  /**
   * Returns the installation directory of CLAN. If CLAN is not found
   * then it will call abort().
   *
   * @return path of CLAN's install dir.
   */
  static const std::string& getCLANPath();

  /**
   * Returns a string with the current username.
   */
  static const std::string& getUserName();
};

} // namespace util
} // namespace cc

#endif // __CODECOMPASS_UTIL_ENVIRONMENT_H__
