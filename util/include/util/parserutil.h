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
 * Recursively iterate over the given directory
 * @param path_ - directory or a regular file
 * @param callback_ - callback function which will be called on each existing path_
 * @return false if the iteration stops, otherwise true
 */
bool iterateDirectoryRecursive(
  const std::string& path_,
  DirIterCallback callback_);

} // util
} // cc

#endif // CC_UTIL_PARSEUTIL_H
