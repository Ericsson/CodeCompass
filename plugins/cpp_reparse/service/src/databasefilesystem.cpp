#include <cerrno>
#include <chrono>
#include <deque>
#include <functional>

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>

#include "databasefilesystem.h"

using namespace llvm;
using namespace clang::vfs;

namespace
{

using namespace cc;

typedef odb::query<model::File> FileQuery;
typedef odb::result<model::File> FileResult;
typedef odb::query<model::FileContentLength> FileContentLengthQuery;
typedef odb::result<model::FileContentLength> FileContentLengthResult;

/**
 * Retrieve a file by path from the database.
 */
model::FilePtr getFile(
  odb::database& db_,
  const std::string& path_)
{
  model::FilePtr file;
  util::OdbTransaction {db_} ([&]() {
    file = db_.query_one<model::File>(FileQuery::path == path_);
  });
  return file;
}

/**
 * Retrieve the children of the given path from the database.
 */
std::vector<model::FilePtr> getChildrenOfFile(
  odb::database& db_,
  const model::FileId& fileId_)
{
  std::vector<model::FilePtr> result;
  util::OdbTransaction {db_} ([&]() {
    FileResult res = db_.query<model::File>(FileQuery::parent == fileId_);
    for (auto fp : res)
      result.push_back(std::make_shared<model::File>(std::move(fp)));
  });
  return result;
}

Status fileToStatus(
  odb::database& db_,
  const model::File& file_)
{
  using namespace llvm::sys::fs;

  file_type fileType = file_type::type_unknown;
  if (file_.type == model::File::DIRECTORY_TYPE)
    fileType = file_type::directory_file;
  else if (file_.type == model::File::BINARY_TYPE)
    fileType = file_type::block_file;
  else if (file_.type == "CPP")
    fileType = file_type::regular_file;

  size_t size = 0;
  if (file_.content)
    util::OdbTransaction {db_} ([&]() {
      // For some weird thing, query_one produces an error here if one attempts
      // to use a std::shared_ptr as the result of the query_one().
      FileContentLengthResult res = db_.query<model::FileContentLength>(
        FileContentLengthQuery::hash == file_.content.object_id());

      if (!res.empty())
        size = res.begin()->size;
    });

  return Status(file_.path, UniqueID(0, file_.id),
                llvm::sys::toTimePoint(file_.timestamp), 0, 0,
                size, fileType, perms::all_read);
}

/**
 * A Clang VFS directory iterator over the children of a directory that is
 * stored in the database.
 */
class DatabaseDirectoryIterator : public clang::vfs::detail::DirIterImpl
{
public:
  DatabaseDirectoryIterator(odb::database& db_,
                            const model::FilePtr& dir_,
                            std::error_code& ec_)
    : _db(db_)
  {
    if (dir_->type != model::File::DIRECTORY_TYPE)
    {
      ec_ = std::error_code(EIO, std::generic_category());
      return;
    }

    for (const auto& fp : getChildrenOfFile(db_, dir_->id))
      _remainingEntries.push_back(fp);

    // This sets the iterator's current element to the first one, if exists.
    ec_ = increment();
  }

  virtual ~DatabaseDirectoryIterator() = default;

  std::error_code increment() override
  {
    if (_remainingEntries.empty())
    {
      CurrentEntry = Status();
      return std::error_code(ENOENT, std::generic_category());
    }

    CurrentEntry = fileToStatus(_db, *_remainingEntries.front());
    _remainingEntries.pop_front();
    return std::error_code();
  }

private:
  odb::database& _db;
  std::deque<model::FilePtr> _remainingEntries;
};

/**
 * A Clang VFS File that is created from contents that reside in the database.
 */
class DatabaseFile : public File
{
public:
  DatabaseFile(const Status& status_, std::string&& content_)
    : _status(status_),
      _content(std::move(content_))
  {
    assert(_content.size() == status_.getSize() && "The content's size should "
           "be the same as known by the status.");
  }

  virtual ~DatabaseFile()
  {
    close();
  }

  ErrorOr<Status> status() override
  {
    return _status;
  }

  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(
    const Twine& /*name_*/,
    int64_t /*fileSize_*/,
    bool /*requiresNullTerminator_*/,
    bool /*isVolatile_*/) override
  {
    // The buffer is copied into a MemoryBuffer that owns the copied contents,
    // because Clang seems to call close() on the File entry *before* giving it
    // to the parser, which means the instance's _content is destroyed early.
    return MemoryBuffer::getMemBufferCopy(_content, _status.getName());
  }

  std::error_code close() override
  {
    return std::error_code();
  }

private:
  const Status _status;
  const std::string _content;
};

} // namespace (anonymous)


namespace cc
{
namespace service
{
namespace reparse
{

DatabaseFileSystem::DatabaseFileSystem(std::shared_ptr<odb::database> db_)
  : _db(db_),
    _transaction(db_),
    _currentWorkingDirectory("/")
{}

ErrorOr<Status> DatabaseFileSystem::status(const Twine& path_)
{
  model::FilePtr file = getFile(*_db, path_.str());
  if (!file)
    return std::error_code(ENOENT, std::generic_category());

  return fileToStatus(*_db, *file);
}

ErrorOr<std::unique_ptr<File>>
DatabaseFileSystem::openFileForRead(const Twine& path_)
{
  model::FilePtr file = getFile(*_db, path_.str());
  if (!file)
    return std::error_code(ENOENT, std::generic_category());
  if (file->type == model::File::DIRECTORY_TYPE)
    return std::error_code(EISDIR, std::generic_category());
  if (file->type == model::File::UNKNOWN_TYPE || !file->content)
    return std::error_code(EIO, std::generic_category());

  std::unique_ptr<File> dbFile;
  _transaction([&](){
    dbFile = std::make_unique<DatabaseFile>(
      fileToStatus(*_db, *file),
      std::move(file->content.load()->content));
  });
  return dbFile;
}

directory_iterator
DatabaseFileSystem::dir_begin(const Twine& dir_, std::error_code& ec_)
{
  model::FilePtr dirFile = getFile(*_db, dir_.str());
  if (dirFile &&  dirFile->type == model::File::DIRECTORY_TYPE)
    return directory_iterator(std::make_shared<DatabaseDirectoryIterator>(
      *_db, dirFile, ec_));

  // If the folder does not exist, or isn't a folder, return an end-iterator.
  return directory_iterator();
}

ErrorOr<std::string> DatabaseFileSystem::getCurrentWorkingDirectory() const
{
  return _currentWorkingDirectory;
}

std::error_code
DatabaseFileSystem::setCurrentWorkingDirectory(const Twine& path_)
{
  SmallString<128> path(path_.str());
  std::error_code error = sys::fs::make_absolute(_currentWorkingDirectory,
                                                 path);

  if (!error)
    _currentWorkingDirectory = path.str().str();

  getCurrentWorkingDirectory();
  return error;
}

} // namespace reparse
} // namespace service
} // namespace cc
