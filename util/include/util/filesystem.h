#ifndef CC_UTIL_FILESYSTEM_H
#define	CC_UTIL_FILESYSTEM_H

#include <string>

namespace cc 
{
namespace util 
{

/**
 * \brief convert path to its shortest
 *
 */
std::string unifyPath(std::string path_);

// get the current directory
std::string getCurrentDirectory();
// get path from a specific directory
std::string pathFromDirectory(const std::string directory_, const std::string path_);
// get path from the current directory
std::string pathFromCurrentDirectory(const std::string path_);

// get parts of the path
std::string getPathRoot(const std::string & path_);
std::string getPathParent(const std::string & path_);
std::string getPathFilename(const std::string & path_);
std::string getPathExtension(const std::string & path_);
std::string getFilenameWithoutExtension(std::string path);

// ordering of path (only valid for unified paths)
bool lesserPath(const std::string & left_, const std::string & right_);
bool equalPath(const std::string & left_, const std::string & right_);

// relative and absolute path
bool isRelativePath(const std::string & path_);
bool isAbsolutePath(const std::string & path_);

// functions for Windows file system
std::string unifyWindowsPath(const std::string & path_, bool uniCase = false);
std::string windowsPathFromDirectory(const std::string directory_, const std::string path_);
std::string getWindowsPathRoot(const std::string & path_);
std::string getWindowsPathParent(const std::string & path_);
std::string getWindowsPathFilename(const std::string & path_);
std::string getWindowsPathExtension(const std::string & path_);
bool lesserWindowsPath(const std::string & left_, const std::string & right_);
bool equalWindowsPath(const std::string & left_, const std::string & right_);
bool isRelativeWindowsPath(const std::string & path_);
bool isAbsoluteWindowsPath(const std::string & path_);

// functions for Unix/Linux file system
std::string unifyUnixPath(std::string path_);
std::string unixPathFromDirectory(const std::string directory_, const std::string path_);
std::string getUnixPathRoot(const std::string & path_);
std::string getUnixPathParent(const std::string & path_);
std::string getUnixPathFilename(const std::string & path_);
std::string getUnixPathExtension(const std::string & path_);
bool lesserUnixPath(const std::string & left_, const std::string & right_);
bool equalUnixPath(const std::string & left_, const std::string & right_);
bool isRelativeUnixPath(const std::string & path_);
bool isAbsoluteUnixPath(const std::string & path_);

// TODO: directory iterator and file status to remove llvm dependency

} // util
} // cc


#endif // CC_UTIL_FILESYSTEM_H

