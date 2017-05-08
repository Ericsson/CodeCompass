#ifndef CC_UTIL_PARSEUTIL_H
#define CC_UTIL_PARSEUTIL_H

namespace cc
{
namespace util
{

/**
 * Callback function type for iterateDirectoryRecursive.
 *
 * The parameter is the full path of the current entity.
 * If the callback returns false, then the iteration stops.
 */
typedef std::function<bool (const std::string&)> DirIterCallback;

/**
 * Recursively iterate over the given directory.
 * @param path_ Directory or a regular file.
 * @param callback_ Callback function which will be called on each existing
 * path_. If this callback returns false then the files under the current
 * directory won't be iterated.
 * @return false if the callback_ returns false on the given path_.
 */
bool iterateDirectoryRecursive(
  const std::string& path_,
  DirIterCallback callback_);

} // util
} // cc

#endif // CC_UTIL_PARSEUTIL_H
