#ifndef PARSER_INTERNAL_SOURCE_MANAGER_H
#define PARSER_INTERNAL_SOURCE_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <magic.h>

#include <parser/commondefs.h>

#include <model/file.h>
#include <model/project.h>

#include <util/odbobjectcache.h>

namespace cc
{
  
namespace model
{
  class Workspace;
}

namespace parser
{

/**
 * Helper class for adding / getting files.
 *
 * This class is thread safe!
 */
class SourceManager
{
public:
  ///
  /// Flags for addFile and getCreateFile.
  ///
  enum CreationFlags
  {
    Defaults      = 0x0000, //< Do everything as usual
    WithContent   = 0x0000, //< Add file with content
    NoContent     = 0x0001, //< Add file without content
    Directory     = 0x0002, //< The given path is a directory
    SkipPathUnify = 0x0004  //< Skip path unification
  };

  /**
   * Constructs a SourceManager.
   *
   * @param ws_ a workspace.
   * @param props_ common parser properties (for project).
   */
  SourceManager(std::shared_ptr<model::Workspace> ws_, ParseProps& props_);

  /**
   * Destructs a SourceManager.
   */
  ~SourceManager();

  /**
   * Adds a file to the database. If the file is already persisted in the
   * database then it will returns true.
   *
   * If the file is not exists on the disc and the NoContent flag is not
   * presents, then it will return true.
   *
   * @param path_ file path.
   * @param flag_ flags.
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return true if the file successfully added or already in the database.
   */
  bool addFile(
    const std::string& path_,
    int flag_ = Defaults,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

  /**
   * A combination of addFile and findFile. First it tries to lookup the given
   * file. If it failed then tries to add (as addFile).
   *
   * If it returns false then the file_ smart pointer is nullptr.
   *
   * @param path_ file path.
   * @param file_ a smart pointer for the result.
   * @param flag_ flags.
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return true if file_ pointer is set successfully.
   */
  bool getCreateFile(
    const std::string& path_,
    model::FilePtr& file_,
    int flag_ = Defaults,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

  /**
   * Finds a file in the database.
   *
   * If it returns false then the file_ smart pointer is nullptr.
   *
   * @param path_ file path.
   * @param file_ a smart pointer for the result.
   * @param flag_ flags.
   * @return true if file_ pointer is set successfully.
   */
  bool findFile(
    const std::string& path_,
    model::FilePtr& file_,
    int flag_ = Defaults | NoContent);
  
  /**
   * Determines whether the given file is plain text or not.
   *
   * @param path_ file path to check
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return true if the file is plain text, false otherwise
   */
  bool isPlainText(
    const std::string& path_,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

private:
  /**
   * Returns true if the given file is a text file by its "magic".
   *
   * It's safe to call this method from different threads.
   *
   * @param path_ file path to check
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return true if the given file is a text file.
   */
  bool isTextMagic(
    const std::string& path_,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

  /**
   * This is the "locked" version of getCreateFile.
   *
   * Should be called from a transaction with a locked _createFileMutex.
   *
   * @param path_ file path.
   * @param file_ a smart pointer for the result.
   * @param flag_ flags.
   * @return true if file_ pointer is set successfully.
   */
  model::FilePtr tryGetFile(const std::string& path_);

  /**
   * Creates a file entry.
   *
   * This method isn't thread safe.
   *
   * @param path_ file path (in canonical form).
   * @param flag_ flags.
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return a new file entry.
   */
  model::FilePtr createFileEntry(
    const std::string& path_,
    int flag_,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

  /**
   * Creates a new file content entry but it won't persist anything.
   *
   * This method its thread safe.
   *
   * @param path_ file path (in canonical form).
   * @param buffer_ optional buffer for file content.
   * @param bufferSize_ size (character count) of the buffer.
   * @return a file content instance. The content itself is empty when it is
   *         persisted before.
   */
  model::FileContentPtr createFileContent(
    const std::string& path_,
    const char* buffer_ = nullptr,
    std::size_t bufferSize_ = 0);

  /**
   * Gets an object instance for the parent of the given path.
   *
   * This method is not thread safe.
   *
   * @param path_ a path.
   * @return the parent file.
   */
  model::FilePtr getParent(const std::string& path_);

  /**
   * The database.
   */
  std::shared_ptr<model::Workspace> _ws;

  /**
   * Parse properties (project).
   */
  ParseProps _props;

  /**
   * A cache for file objects.
   */
  util::OdbObjectCache<std::string, std::shared_ptr<model::File>> _fileCache;

  /**
   * A cache for file content hashes.
   */
  std::unordered_set<std::string> _fileContentHashes;

  /**
   * Libmagic cookie. It can be null if loading the magic file failed.
   */
  magic_t _magicCookie;

  /**
   * Libmagic is not thread safe so we have to lock before using it.
   */
  std::mutex _magicCookieMutex;

  /**
   * Mutex for serializing file creation commits.
   *
   * We need to serialize the commits otherwise we might get foreign key error
   * in OdbObjectCache.
   */
  std::mutex _createFileMutex;
};

} //parser
} //namespace cc

#endif //PARSER_INTERNAL_SOURCE_MANAGER_H
