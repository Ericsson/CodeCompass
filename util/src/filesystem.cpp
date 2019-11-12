#include <cstdlib>

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

  if (fs::exists(fs::system_complete(fs::path(path))))
  {
    return fs::canonical(fs::system_complete(fs::path(path)))
      .parent_path().parent_path().string();
  }
  else
  {
    std::string pPath;
    std::string fullPath = std::getenv("PATH");
    std::string delimiter = ":";

    std::size_t pos = 0;
    while ((pos = fullPath.find(delimiter)) != std::string::npos)
    {
      pPath = fullPath.substr(0, pos) + "/" + path;

      if (fs::exists(fs::path(pPath)))
      {
        return fs::canonical(fs::path(pPath))
          .parent_path().parent_path().string();
      }

      fullPath.erase(0, pos + delimiter.length());
    }
  }

  throw std::runtime_error(std::string("Could not find ") + path + std::string("."));
}

} // namespace util
} // namespace cc
