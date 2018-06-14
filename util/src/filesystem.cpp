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
  return fs::canonical(fs::system_complete(fs::path(path)))
    .parent_path().parent_path().string();
}

} // namespace util
} // namespace cc
