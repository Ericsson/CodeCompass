#ifndef CC_UTIL_ENVIRONMENT_H
#define CC_UTIL_ENVIRONMENT_H

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
};

} // namespace util
} // namespace cc

#endif // CC_UTIL_ENVIRONMENT_H
