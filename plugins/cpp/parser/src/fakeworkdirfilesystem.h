#ifndef CC_PARSER_FAKEWORKDIRFILESYSTEM_H
#define CC_PARSER_FAKEWORKDIRFILESYSTEM_H

#include <llvm/Support/VirtualFileSystem.h>

namespace cc
{
namespace parser
{

/** A Clang Virtual File System implementation that emulates chdir in a
 * thread-safe way, and without the need for the working directory to exist.
 * It proxies calls to another file system, modifying path arguments to be
 * absolute based on the working directory of this one.
 */
class FakeWorkDirFileSystem : public llvm::vfs::FileSystem
{
public:
  explicit FakeWorkDirFileSystem(llvm::IntrusiveRefCntPtr<FileSystem> FS_)
    : _FS(std::move(FS_))
  {}

  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine& path_) override
  {
    return _FS->status(toAbsolute(path_));
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> openFileForRead(
    const llvm::Twine& path_) override
  {
    return _FS->openFileForRead(toAbsolute(path_));
  }

  llvm::vfs::directory_iterator dir_begin(
    const llvm::Twine& dir_,
    std::error_code& ec_) override
  {
    return _FS->dir_begin(toAbsolute(dir_), ec_);
  }

  std::error_code getRealPath(
    const llvm::Twine& path_,
    llvm::SmallVectorImpl<char>& output_) const override
  {
    return _FS->getRealPath(toAbsolute(path_), output_);
  }

  std::error_code isLocal(const llvm::Twine& path_, bool& result_) override
  {
    return _FS->isLocal(toAbsolute(path_), result_);
  }
  
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override
  {
    return _workDir;
  }

  std::error_code setCurrentWorkingDirectory(const llvm::Twine& path_) override
  {
    // We don't verify if the working directory exists on purpose.
    _workDir = path_.str();
    return std::error_code();
  }

private:
  llvm::IntrusiveRefCntPtr<FileSystem> _FS;
  std::string _workDir;

  llvm::SmallString<256> toAbsolute(const llvm::Twine& relPath_) const
  {
    llvm::SmallString<256> absPath;
    relPath_.toVector(absPath);
    llvm::sys::fs::make_absolute(_workDir, absPath);
    return absPath;
  }
};

} // parser
} // cc

#endif // CC_PARSER_FAKEWORKDIRFILESYSTEM_H
