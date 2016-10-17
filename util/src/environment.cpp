#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <deque>
#include <unistd.h>
#include <pwd.h>
#include <cstdio>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <util/environment.h>

namespace fs = boost::filesystem;

namespace
{

enum DirLayout
{
  Standard,
  SourceTree
};

/**
 * Global variable for CodeCompass install root directory path.
 */
std::string globEnvRoot;

std::string globUserName;

class EnvList
{
public:
  using Values = std::deque<std::string>;

  EnvList(const char* name_, const char* separator_) :
    _name(name_),
    _separator(separator_)
  {
    get();
  }

public:
  void get()
  {
    const char* value = std::getenv(_name);
    if (value)
    {
      boost::split(_values, value, boost::is_any_of(_separator));
    }
  }

  void set()
  {
    std::string value = boost::join(_values, _separator);
    ::setenv(_name, value.c_str(), 1);
  }

  Values& values()
  {
    return _values;
  }

private:
  const char* _name;
  const char* _separator;
  Values _values;
};

/**
 * Returns the path of the current executable file.
 */
std::string getExecutablePathOrDie()
{
  char path[PATH_MAX];

  ssize_t pathLength = ::readlink("/proc/self/exe", path, PATH_MAX);
  if (pathLength == -1)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to get exec path!";
    ::exit(-1);
  }

  return std::string(path, static_cast<std::size_t>(pathLength));
}

/**
 * Returns the CodeCompass install root.
 */
std::string getRoot(DirLayout& layout_)
{
  std::string root;

  layout_ = Standard;

  fs::path exePath(getExecutablePathOrDie());
  fs::path exeDirPath = exePath.parent_path();
  std::string exeDirPathStr = exeDirPath.string();

  if (exeDirPathStr.rfind("bin") == exeDirPathStr.size() - 3)
  {
    // It seems it is an executable in a regular installation folder
    root = exeDirPath.parent_path().string();
  }
  else if (exeDirPathStr.rfind(".libs") == exeDirPathStr.size() - 5)
  {
    // Possibly a make check or something...
    layout_ = SourceTree;

    // Find source tree root
    while (exeDirPath.string() != "/")
    {
      exeDirPath = exeDirPath.parent_path();

      std::string ftest = exeDirPath.string() + "/config.status";
      if (::access(ftest.c_str(), F_OK) == 0)
      {
        root = exeDirPath.string();
        break;
      }
    }

    if (exeDirPath == "/")
    {
      exeDirPath.clear();
    }
  }

  if (root.empty())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to detect installation path!";
    ::exit(-1);
  }

  if (root.back() == '/')
  {
    root.pop_back();
  }

  BOOST_LOG_TRIVIAL(debug) << "Detected CodeCompass install path: " << root;

  return root;
}

}

namespace cc
{
namespace util
{

void Environment::init()
{
  DirLayout rootDirLayout;
  globEnvRoot = ::getRoot(rootDirLayout);

  EnvList path("PATH", ":");
  EnvList classpath("CLASSPATH", ":");

  switch (rootDirLayout)
  {
    case Standard:
    {
      // FIXME: maybe it is unnecessary
      path.values().push_front(globEnvRoot + "/bin");

      classpath.values().push_front(globEnvRoot + "/lib/*");
      classpath.values().push_front(globEnvRoot + "/lib/java/*");
      break;
    }
    case SourceTree:
    {
      classpath.values().push_front(globEnvRoot + "/lib/java/*");
      break;
    }
  }

  path.set();
  classpath.set();

  // read the username of the current user
  {
    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    if(pw)
    {
      globUserName = pw->pw_name;
    }
  }
}


} // namespace util
} // namespace cc

