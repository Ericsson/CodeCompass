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

} // namespace util
} // namespace cc

#endif
