#ifndef __CC_UTIL_CRASH_PROTECT_H__
#define __CC_UTIL_CRASH_PROTECT_H__

#include <model/buildaction.h>

namespace cc
{
namespace parser
{
namespace crash
{

/**
 * Inits the crash handler. The current implementation sets a standard UNIX
 * signal handler for SIGSEGV.
 */
void initHandler();

/**
 * Low-level API for setting the build action for the current thread. If the
 * application gets a SIGSEGV, the action id is written to the standard output.
 * You can unset the action by giving a null pointer.
 *
 * Probably you don't need to use this function directly, instead use the
 * ScopedProtection class.
 *
 * @param action_ the action in context.
 * @param contextText_ extra text for diagnostics
 */
void setAction(model::BuildActionPtr action_, std::string contextText_);

/**
 * Helper class for protecting a parsing procedure for a build action.
 */
class ScopedProtection
{
public:
  /**
   * Constructs a new ScopedProtection for the given action. The constructor
   * sets the action using the setAction(model::BuildActionPtr) function.
   *
   * @param action_ the action in context.
   */
  ScopedProtection(model::BuildActionPtr action_, std::string contextText_ = "")
  {
    setAction(action_, std::move(contextText_));
  }

  /**
   * Unsets the current build action using the setAction(model::BuildActionPtr)
   * function.
   */
  ~ScopedProtection()
  {
    setAction(nullptr, "");
  }

  ScopedProtection(const ScopedProtection&) = delete;
};

} // crash
} // parser
} // cc

#endif // __CC_UTIL_CRASH_PROTECT_H__
