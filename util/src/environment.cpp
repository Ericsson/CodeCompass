#include <util/environment.h>
#include <util/filesystem.h>
#include <util/streamlog.h>

#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <deque>
#include <unistd.h>
#include <pwd.h>
#include <cstdio>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

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

std::string globCLANPath;

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
    SLog(cc::util::CRITICAL) << "Failed to get exec path!";
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

  std::string exeDirPath = cc::util::getPathParent(getExecutablePathOrDie());

  if (exeDirPath.rfind("bin") == exeDirPath.size() - 3)
  {
    // It seems it is an executable in a regular installation folder
    root = cc::util::getPathParent(exeDirPath);
  }
  else if (exeDirPath.rfind(".libs") == exeDirPath.size() - 5)
  {
    // Possibly a make check or something...
    layout_ = SourceTree;

    // Find source tree root
    while (exeDirPath != "/")
    {
      exeDirPath = cc::util::getPathParent(exeDirPath);

      std::string ftest = exeDirPath + "/config.status";
      if (::access(ftest.c_str(), F_OK) == 0)
      {
        root = exeDirPath;
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
    SLog(cc::util::CRITICAL) << "Failed to detect installation path!";
    ::exit(-1);
  }

  if (root.back() == '/')
  {
    root.pop_back();
  }

  SLog(cc::util::INFO) << "Detected CodeCompass install path: " << root;

  return root;
}

/**
 * Returns the installation path of CLAN or empty string if not found.
 */
std::string detectCLAN(const std::string& root_)
{
  std::string path = root_ + "/CLAN";
  if (access(path.c_str(), R_OK | X_OK) == 0)
  {
    // It is a normal package => CLAN is under the installation root.
    return path;
  }

  const char* depsRoot = ::getenv("CCMP_DEPS");
  if (depsRoot)
  {
    path  = depsRoot;
    path += "/CLAN";
    if (access(path.c_str(), R_OK | X_OK) == 0)
    {
      // CLAN is under the deps folder.
      return path;
    }
  }

  // CLAN is not found (it could be OK).
  return "";
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
      classpath.values().push_front(globEnvRoot + "/share/codecompass/java/*");
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

  globCLANPath = detectCLAN(globEnvRoot);
}

const std::string& Environment::getInstallPath()
{
  return globEnvRoot;
}

const std::string& Environment::getCLANPath()
{
  if (globCLANPath.empty())
  {
    assert(false && "CLAN must be installed!");
    abort();
  }

  return globCLANPath;
}

const std::string& Environment::getUserName()
{
  return globUserName;
}

} // namespace util
} // namespace cc

