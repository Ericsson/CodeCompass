#include <chrono>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include <util/parserutil.h>

namespace fs = boost::filesystem;

namespace
{

/**
* Context to iterateDirectoryRecursive.
*/
struct TraverseContext
{
 /**
  * Time of last status report.
  */
 std::chrono::time_point<std::chrono::system_clock> lastReportTime =
   std::chrono::system_clock::now();
 /**
  * Total number of (regular) files visited in the iteration.
  */
 std::size_t numFilesVisited = 0;
 /**
  * Total number of directories visited in the iteration.
  */
 std::size_t numDirsVisited = 0;
};

bool iterateDirectoryRecursive(
  TraverseContext& context_,
  const std::string& path_,
  cc::util::DirIterCallback callback_)
{
  //--- Status reporting ---//

  auto currTime = std::chrono::system_clock::now();
  if ((currTime - context_.lastReportTime) >= std::chrono::seconds(15))
  {
    // It's time to report
    BOOST_LOG_TRIVIAL(info)
      << "Recursive directory iteration: visited "
      << context_.numFilesVisited << " files in "
      << context_.numDirsVisited << " directories so far.";
    context_.lastReportTime = currTime;
  }

  fs::path p(path_);

  if (!fs::exists(p))
  {
    BOOST_LOG_TRIVIAL(warning) << "Not found: " << p;
    return true;
  }

  if (fs::is_directory(p))
  {
    ++context_.numDirsVisited;
  }
  else if(fs::is_regular_file(p))
  {
    ++context_.numFilesVisited;
  }

  //--- Call callback ---//

  if (!callback_(path_))
    return false;

  //--- Iterate over directory content ---//

  if (fs::is_directory(p))
  {
    fs::directory_iterator end_iter;
    fs::directory_iterator it(p);
    for (; it != end_iter; ++it)
    {
      std::string path = it->path().c_str();

      if (!iterateDirectoryRecursive(context_, path , callback_))
      {
        // Stop iteration
        return false;
      }
    }
  }

  return true;
}

} // anonymus namespace

namespace cc
{
namespace util
{

bool iterateDirectoryRecursive(
  const std::string& path_,
  DirIterCallback callback_)
{
  TraverseContext ctx;
  std::string path = path_;
  return ::iterateDirectoryRecursive(ctx, path, callback_);
}

}
}
