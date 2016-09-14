// include header
#include <util/filesystem.h>

// includes
#include <memory>
#include <list>
#include <errno.h>

// platform includes
#if defined(_WIN32)
  #include <direct.h>
#elif defined(__unix)
  #include <unistd.h>
#endif

namespace cc
{
namespace util
{

// anonymous namespace for path string collation
namespace {

  // path character lesser function for unix
  inline bool unixCharLesser(char left_, char right_)
  {
    // if both are separators, the function needs to return false
    if (right_ == '/') return false;
    if (left_ == '/') return true;
    // separators are the characters with the less value
    return left_ < right_;
  }

  inline bool windowsCharLesser(char left_, char right_)
  {
    // if both are separators, the function needs to return false
    if ((right_ == '/') || (right_ == '\\')) return false;
    if ((left_ == '/') || (left_ == '\\')) return true;
    // case insensitive lesser, characters ordered as if they were all uppercase
    if ((left_ >= 'a') && (left_ <= 'z')) left_ += 'A' - 'a';
    if ((right_ >= 'a') && (right_ <= 'z')) right_ += 'A' - 'a';
    return left_ < right_;
  }

  // case insensitive equal function
  inline bool windowsCharEqual(char left_, char right_)
  {
    return (left_ == right_)
    || ((left_ >= 'A') && (left_ <= 'Z') && (right_ == left_ + ('a' - 'A')))
    || ((right_ >= 'A') && (right_ <= 'Z') && (left_ == right_ + ('a' - 'A')));
  }

  inline bool unixStringLesser(const std::string & left_, const std::string & right_)
  {
    std::size_t i = 0;
    // go thought the common part of the string
    while ((i < left_.size()) && (i < right_.size()))
    {
      // and if there is a difference use lesser
      if (left_[i] != right_[i]) return unixCharLesser(left_[i], right_[i]);
    }
    // lesser, when the right side remains
    return i < right_.size();
  }

  inline bool windowsStringLesser(const std::string & left_, const std::string & right_)
  {
    std::size_t i = 0;
    // go thought the common part of the string
    while ((i < left_.size()) && (i < right_.size()))
    {
      if (!windowsCharEqual(left_[i], right_[i])) return windowsCharLesser(left_[i], right_[i]);
    }
    // lesser only, when the right string longer
    return i < right_.size();
  }

  inline bool windowsStringEqual(const std::string & left_, const std::string & right_)
  {
    std::size_t i = 0;
    // go thought the common part of the string
    while ((i < left_.size()) && (i < right_.size()))
    {
      if (!windowsCharEqual(left_[i], right_[i])) return false;
    }
    // lesser, when the right side remains
    return left_.size() == right_.size();
  }

  std::size_t unixGetRootSize(const std::string & path_)
  {
    // the unix root is an optional '/'
    return ((path_.size() > 0) && (path_[0] == '/')) ? 1 : 0;
  }

  std::size_t windowsGetRootSize(const std::string & path_)
  {
    // the format of the windows root ["//" ["?/" | "./"]] | ["c:"] ["/"], plus each "/" can be repeated
    std::size_t length = 0;
    // check for the "c:" part
    if ((path_.size() > 1) && (path_[1] == ':')) length = 2;
    // skip any number of '/'
    while ((length < path_.size()) && ((path_[length] == '\\') || (path_[length] == '/'))) ++ length;
    // if path is "\\" see if it's "\\.\" or "\\?\"
    if (length == 2)
    {
      // check prefix
      const std::string & prefix = path_.substr(0,4);
      // check "\\?\"
      if ((prefix == "\\\\?\\") || (prefix == "\\\\.\\"))
      {
        // skip any other slash
        length = 4;
        while ((length < path_.size()) && ((path_[length] == '\\') || (path_[length] == '/'))) ++ length;
      }
    }
    return length;
  }

// close anonymous namespace
}

// set default functions
#if defined(_WIN32)

std::string unifyPath(std::string path_) {return unifyWindowsPath(std::move(path_));}
std::string pathFromDirectory(const std::string directory_, const std::string path_) {return windowsPathFromDirectory(directory_, path_);}
std::string getPathRoot(const std::string & path_) {return getWindowsPathRoot(path_);}
std::string getPathParent(const std::string & path_) {return getWindowsPathParent(path_);}
std::string getPathFilename(const std::string & path_) {return getWindowsPathFilename(path_);}
std::string getPathExtension(const std::string & path_) {return getWindowsPathExtension(path_);}
bool lesserPath(const std::string & left_, const std::string & right_) {return lesserWindowsPath(left_, right_);}
bool equalPath(const std::string & left_, const std::string & right_) {return equalWindowsPath(left_, right_);}
bool isRelativePath(const std::string & path_) {return isRelativeWindowsPath(path_);}
bool isAbsolutePath(const std::string & path_) {return isAbsoluteWindowsPath(path_);}

#elif defined(__unix)

std::string unifyPath(std::string path_) {return unifyUnixPath(std::move(path_));}
std::string pathFromDirectory(const std::string directory_, const std::string path_) {return unixPathFromDirectory(directory_, path_);}
std::string getPathRoot(const std::string & path_) {return getUnixPathRoot(path_);}
std::string getPathParent(const std::string & path_) {return getUnixPathParent(path_);}
std::string getPathFilename(const std::string & path_) {return getUnixPathFilename(path_);}
std::string getPathExtension(const std::string & path_) {return getUnixPathExtension(path_);}
bool lesserPath(const std::string & left_, const std::string & right_) {return lesserUnixPath(left_, right_);}
bool equalPath(const std::string & left_, const std::string & right_) {return equalUnixPath(left_, right_);}
bool isRelativePath(const std::string & path_) {return isRelativeUnixPath(path_);}
bool isAbsolutePath(const std::string & path_) {return isAbsoluteUnixPath(path_);}

#endif

// functions for the "current" system
#if defined(_WIN32) || defined(__unix)

std::string getCurrentDirectory()
{
  // getcwd is pretty much unusable, the program needs to "guess" the right
  // buffer size. This function starts with 256 bytes and doubles the buffer on
  // each failure until 64 kilobytes, witch is way past the 260 character limit
  // for windows anyways (249 character for directories).
  std::size_t length = 256;
  while (length <= 65536)
  {
    std::unique_ptr<char[]> buffer(new char[length]);
#ifdef _WIN32
    char * r = _getcwd(buffer, length);
#else
    char * r = getcwd(buffer.get(), length);
#endif
    // upon successful execution
    if (r)
    {
      std::string result = buffer.get();
      return result;
    }
    else
    {
      // exit if the error is not the buffer being too short
      if (errno != ERANGE) return "";
      // double the buffer, and see if it solves the problem
      length *= 2;
    }
  }
  // if all attempts failed return an empty string
  return "";
}

// end of functions
#endif

std::string pathFromCurrentDirectory(const std::string path_)
{
  return pathFromDirectory(getCurrentDirectory(), path_);
}


std::string unifyWindowsPath(const std::string & path_, bool uniCase)
{
  // TODO:
  return "";
}

std::string getFilenameWithoutExtension(std::string path)
{
  auto per = path.rfind('/');
  if (std::string::npos != per)
  {
    path.erase(0, per + 1);
  }

  auto dot = path.find('.');
  if (std::string::npos != dot)
  {
    path.erase(dot);
  }

  return path;
}


std::string unifyUnixPath(std::string path)
{
  const char updir[] = "../";
  const char currdir[] = "./";

  const unsigned int updirLen = sizeof(updir) - 1;
  const unsigned int currdirLen = sizeof(currdir) - 1;

  // remove all "./"
  std::size_t index = 0;
  while (true)
  {
    auto cInd = path.find(currdir, index);
    if (cInd == std::string::npos)
      break;

    if (path[cInd - 1] != '.')
    {
      path.erase(cInd, currdirLen);
      index = cInd;
    } else
    {
      index = cInd + currdirLen;
    }
  }

  // remove all "//"
  index = 0;
  while (true)
  {
    auto cInd = path.find("//", index);
    if (cInd == std::string::npos)
      break;

    path.erase(cInd, 1);
    index = cInd;
  }

  // handle "../"
  index = 0;
  while (true)
  {
    auto start = path.find(updir, index);
    if (start == std::string::npos)
      break;

    int cntDotDot = 1;
    auto end = start + updirLen;
    while (path[end] == '.')
    {
      ++cntDotDot;
      end += updirLen;
    }

    if (start == 0)
    {
      index = end;
      continue;
    } else if (start == 1)
    {
      path.erase(start, end - start);
      continue;
    }

    auto eraseStart = path.rfind('/', start - 2);
    --cntDotDot;

    while (cntDotDot--)
    {
      eraseStart = path.rfind('/', eraseStart - 1);
    }
    path.erase(eraseStart, end - eraseStart - 1);
    index = eraseStart;
  }

  return path;
}


// GOOD;
std::string windowsPathFromDirectory(const std::string directory_, const std::string path_)
{
  // empty paths interpreted as "."
  if (directory_ == "") return path_;
  if (path_ == "") return directory_;
  // join paths only when the path is relative
  if (isRelativeWindowsPath(path_))
  {
    return directory_ + "\\" + path_;
  }
  else
  {
    return path_;
  }
}

// GOOD;
std::string unixPathFromDirectory(const std::string directory_, const std::string path_)
{
  // empty paths interpreted as "."
  if (directory_ == "") return path_;
  if (path_ == "") return directory_;
  // join paths only when the path is relative
  if (isRelativeUnixPath(path_))
  {
    return directory_ + "/" + path_;
  }
  else
  {
    return path_;
  }
}

// GOOD;
std::string getWindowsPathRoot(const std::string & path_)
{
  // return the root
  return path_.substr(0, windowsGetRootSize(path_));
}

// GOOD;
std::string getUnixPathRoot(const std::string & path_)
{
  // return the root
  return path_.substr(0, unixGetRootSize(path_));
}

// GOOD;
std::string getWindowsPathParent(const std::string & path_)
{
  // find the separator
  std::size_t i = path_.size() - 1;
  std::size_t root = windowsGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '\\') && (path_[i] != '/')) {-- i;}
  // skip separators and return everything before it
  while ((i != root) && ((path_[i] == '\\') || (path_[i] == '/'))) {-- i;}
  return path_.substr(0, i + 1);
}

// GOOD;
std::string getUnixPathParent(const std::string & path_)
{
  // find the separator
  std::size_t i = path_.size() - 1;
  std::size_t root = unixGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '/')) {-- i;}
  // skip separators and return everything before it
  while ((i != root) && (path_[i] == '/')) {-- i;}
  return path_.substr(0, i + 1);
}

// GOOD;
std::string getWindowsPathFilename(const std::string & path_)
{
  std::size_t i = path_.size() - 1;
  std::size_t root = windowsGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '\\') && (path_[i] != '/')) {-- i;}
  return (i + 1 < path_.size()) ? path_.substr(i + 1, path_.size()) : "";
}

// GOOD
std::string getUnixPathFilename(const std::string & path_)
{
  std::size_t i = path_.size() - 1;
  std::size_t root = unixGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '/')) {-- i;}
  return (i + 1 < path_.size()) ? path_.substr(i + 1, path_.size()) : "";
}

// GOOD;
std::string getWindowsPathExtension(const std::string & path_)
{
  std::size_t i = path_.size() - 1;
  std::size_t root = windowsGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '\\') && (path_[i] != '/') && (path_[i] != '.')) {-- i;}
  return (i + 1 < path_.size()) ? path_.substr(i + 1, path_.size()) : "";
}

// GOOD
std::string getUnixPathExtension(const std::string & path_)
{
  std::size_t i = path_.size() - 1;
  std::size_t root = unixGetRootSize(path_) - 1;
  while ((i != root) && (path_[i] != '/') && (path_[i] != '.')) {-- i;}
  return (i + 1 < path_.size()) ? path_.substr(i + 1, path_.size()) : "";
}

// GOOD;
bool lesserWindowsPath(const std::string & left_, const std::string & right_)
{
  // get the roots
  const std::string & lRoot = getWindowsPathRoot(left_);
  const std::string & rRoot = getWindowsPathRoot(right_);

  // lRoot < rRoot || ...
  if (lRoot != rRoot) return windowsStringLesser(lRoot, rRoot);
  // roots are the same, see the whole string
  return windowsStringLesser(left_, right_);
}

// GOOD;
bool lesserUnixPath(const std::string & left_, const std::string & right_)
{
  // get the roots
  const std::string & lRoot = getUnixPathRoot(left_);
  const std::string & rRoot = getUnixPathRoot(right_);

  // lRoot < rRoot || ...
  if (lRoot != rRoot) return unixStringLesser(lRoot, rRoot);
  // roots are the same, see the whole string
  return unixStringLesser(left_, right_);
}

// GOOD;
bool equalWindowsPath(const std::string & left_, const std::string & right_)
{
  // windows paths needs to ignore the case
  return windowsStringEqual(left_, right_);
}

// GOOD;
bool equalUnixPath(const std::string & left_, const std::string & right_)
{
  // unix paths are equal, if they are lexically equal
  return left_ == right_;
}

bool isRelativeWindowsPath(const std::string & path_)
{
  // relative paths have no root
  return windowsGetRootSize(path_) == 0;
}

bool isRelativeUnixPath(const std::string & path_)
{
  // relative paths have no root
  return unixGetRootSize(path_) == 0;
}

bool isAbsoluteWindowsPath(const std::string & path_)
{
  // absolute paths have full root
  return (windowsGetRootSize(path_) > 2) || ((windowsGetRootSize(path_) == 2) && (path_[1] != ':'));
}

bool isAbsoluteUnixPath(const std::string & path_)
{
  // absolute paths have full root
  return unixGetRootSize(path_) > 0;
}

} // util
} // cc

