#include <cstdlib>
#include <climits>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include <util/filesystem.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace util
{

std::string binaryPathToInstallDir(const char* path)
{
  // Expand the path to a full path as given by the shell. This is then stripped
  // of any symbolic links and relative references.
  // The path is then parented twice (from binary to "bin" folder, and then
  // the root).
  // If the path does not exist, the given binary is searched for in the PATH
  // environment variable.
  // TODO: replace else branch with fs::search_path if Boost 1.64 or above is used.

  if (fs::exists(fs::system_complete(fs::path(path))))
  {
    return fs::canonical(fs::system_complete(fs::path(path)))
      .parent_path().parent_path().string();
  }
  else
  {
    fs::path pPath;
    std::string fullPath = std::getenv("PATH");
    std::string delimiter = ":";

    std::size_t pos = 0;
    while ((pos = fullPath.find(delimiter)) != std::string::npos)
    {
      pPath = fs::path(fullPath.substr(0, pos)) / fs::path(path);

      if (fs::exists(fs::path(pPath)))
      {
        return fs::canonical(pPath)
          .parent_path().parent_path().string();
      }

      fullPath.erase(0, pos + delimiter.length());
    }
  }

  throw std::runtime_error(std::string("Could not find ") + path + std::string("."));
}

std::string findCurrentExecutableDir()
{
  char exePath[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));

  if (len == -1 || len == sizeof(exePath))
    len = 0;

  exePath[len] = '\0';
  return fs::path(exePath).parent_path().string();
}

bool isRootedUnderAnyOf(
  const std::vector<std::string>& paths_,
  const std::string& path_)
{
  auto it = paths_.begin();
  const auto end = paths_.end();
  while (it != end && path_.rfind(*it, 0) != 0)
    ++it;
  return it != end;
}

} // namespace util
} // namespace cc
