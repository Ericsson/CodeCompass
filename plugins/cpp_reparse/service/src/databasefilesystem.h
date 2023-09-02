#ifndef CC_SERVICE_CPPREPARSESERVICE_DATABASEFILESYSTEM_H
#define CC_SERVICE_CPPREPARSESERVICE_DATABASEFILESYSTEM_H

#include <memory>

#include <llvm/Support/VirtualFileSystem.h>

#include <odb/database.hxx>

#include <util/odbtransaction.h>

namespace cc
{
namespace service
{
namespace reparse
{

/**
 * A Clang Virtual File System implementation that retrieves file information
 * and contents from CodeCompass' database.
 */
class DatabaseFileSystem : public llvm::vfs::FileSystem
{
public:
  DatabaseFileSystem(std::shared_ptr<odb::database> db_);

  virtual ~DatabaseFileSystem() = default;

  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine& path_) override;

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const llvm::Twine& path_) override;

  llvm::vfs::directory_iterator
  dir_begin(const llvm::Twine& dir_, std::error_code& ec_) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;

  std::error_code setCurrentWorkingDirectory(const llvm::Twine& path_) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  std::string _currentWorkingDirectory;

  llvm::ErrorOr<std::string> toCanonical(const llvm::Twine& relPath_) const;

  std::string toCanonicalOrSame(const llvm::Twine& relPath_) const;
};

} //namespace reparse
} //namespace service
} //namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_DATABASEFILESYSTEM_H

