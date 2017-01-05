#include "filelocutil.h"

namespace cc
{
namespace parser
{

FileLocUtil::FileLocUtil(SourceManager&               srcMan_,
                         const clang::SourceManager&  clangSrcMan_) :
  FileLocUtilBase(clangSrcMan_),
  _srcMan(srcMan_)
{
}

bool FileLocUtil::setInfo(const clang::SourceLocation& clangLoc_,
                          model::FileLoc&              floc_)
{
  return setPosition(clangLoc_, floc_.range) &&
         setFile(clangLoc_, floc_);
}

bool FileLocUtil::setInfo(const clang::SourceLocation& startLoc_,
                          const clang::SourceLocation& endLoc_,
                          model::FileLoc&              floc_)
{
  return setPosition(startLoc_, endLoc_, floc_.range) &&
         setFile(startLoc_, floc_);
}

bool FileLocUtil::setFile(const clang::SourceLocation& loc_,
                          model::FileLoc&              floc_)
{
  std::string path;
  if (!getFilePath(loc_, path) || path.empty())
  {
    return false;
  }

  model::FilePtr file;
  const char*    fileBuff = nullptr;
  std::size_t    fileBuffSize = 0;

  getBuffer(loc_, fileBuff, fileBuffSize);
  if (!_srcMan.getCreateFile(
        path,
        file,
        SourceManager::Defaults,
        fileBuff, fileBuffSize))
  {
    return false;
  }

  floc_.file = file;

  return true;
}

} // parser
} // cc
