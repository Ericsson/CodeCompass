#ifndef CC_UTIL_FILESYSTEM_H
#define CC_UTIL_FILESYSTEM_H

namespace cc
{
namespace util
{

/**
 * @brief Transform the shell-given path of a CodeCompass binary into the
 * absolute canonical path of CodeCompass' install root.
 *
 * @param path The path of the binary which was started, as given by the
 * executing shell. This should, in most cases be argv[0] of a main() method.
 * @return The installation folder of CodeCompass as a string.
 */
std::string binaryPathToInstallDir(const char* path);

/**
 * @brief Find the directory where a CodeCompass binary is being run from.
 * @return The absolute path of the directory where the binary is located.
 */
std::string findCurrentExecutableDir();

/**
 * @brief Determines if the given path is rooted under
 * any of the given other paths.
 *
 * @param paths_ A list of canonical paths.
 * @param path_ A canonical path to match against the given paths.
 * @return True if any of the paths in paths_ is a prefix of path_,
 * otherwise false.
*/
bool isRootedUnderAnyOf(
  const std::vector<std::string>& paths_,
  const std::string& path_);

} // namespace util
} // namespace cc

#endif
