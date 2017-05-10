#include <fstream>
#include <algorithm>

#include <boost/filesystem.hpp>

#include <util/hash.h>
#include <util/logutil.h>

#include <parser/sourcemanager.h>

namespace cc
{
namespace parser
{

SourceManager::SourceManager(std::shared_ptr<odb::database> db_)
  : _db(db_), _transaction(db_), _magicCookie(nullptr)
{
  _transaction([&, this]() {

    //--- Reload files from database ---//

    for (const model::File& file : db_->query<model::File>())
    {
      _files[file.path] = std::make_shared<model::File>(file);
      _persistedFiles.insert(file.id);
    }

    for (const auto& fileContentId : db_->query<model::FileContentIds>())
      _persistedContents.insert(fileContentId.hash);
  });

  //--- Initialize magic for plain text testing ---//

  if (_magicCookie = ::magic_open(MAGIC_SYMLINK))
  {
    if (::magic_load(_magicCookie, 0) != 0)
    {
      LOG(warning)
        << "libmagic error: "
        << ::magic_error(_magicCookie);

      ::magic_close(_magicCookie);
      _magicCookie = nullptr;
    }
  }
  else
    LOG(warning) << "Failed to create a libmagic cookie!";
}

SourceManager::~SourceManager()
{
  persistFiles();

  if (_magicCookie)
    ::magic_close(_magicCookie);
}

model::FileContentPtr SourceManager::createFileContent(
  const std::string& path_) const
{
  std::ifstream ifs(path_);
  if (!ifs)
  {
    LOG(error) << "Failed to open '" << path_ << "'";
    return nullptr;
  }

  model::FileContentPtr content = std::make_shared<model::FileContent>();

  // Get length of file
  ifs.seekg(0, std::ios::end);
  auto fileSize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  // Get content
  content->content.reserve(fileSize);
  content->content.assign(
    std::istreambuf_iterator<char>(ifs),
    std::istreambuf_iterator<char>());

  // A file may contain 0x00 characters (e.g. in an RTF file). If we store these
  // files in a PostgreSQL database then we get 'invalid byte sequence' errors.
  // FIXME: Convert file content from the file's encoding to the DB's encoding.
  // FIXME: I'm not sure that SPACE character is the best replacement.
  std::replace(content->content.begin(), content->content.end(), '\0', ' ');

  // Generate hash
  content->hash = util::sha1Hash(content->content);

  return content;
}

model::FilePtr SourceManager::getCreateFileEntry(
  const std::string& path_,
  bool withContent_)
{
  //--- Return from cache if it contains ---//

  _createFileMutex.lock();
  std::map<std::string, model::FilePtr>::const_iterator it = _files.find(path_);

  if (it != _files.end())
  {
    model::FilePtr file = it->second;
    _createFileMutex.unlock();
    return file;
  }

  _createFileMutex.unlock();

  //--- Create new file entry ---//

  boost::system::error_code ec;
  boost::filesystem::path path(path_);

  std::time_t timestamp = boost::filesystem::last_write_time(path, ec);
  if (ec)
    timestamp = 0;

  model::FilePtr file(new model::File());
  file->id = util::fnvHash(path_);
  file->path = path_;
  file->timestamp = timestamp;
  file->parent = getCreateParent(path_);
  file->filename = path.filename().native();

  if (boost::filesystem::is_directory(path, ec))
    file->type = model::File::DIRECTORY_TYPE;
  else
    file->type = model::File::UNKNOWN_TYPE;

  if (file->type != model::File::DIRECTORY_TYPE && withContent_)
  {
    if (!boost::filesystem::is_regular_file(path, ec))
    {
      LOG(debug)
        << "'" << path_ << "' is not a regular file! Skip saving content.";
    }
    else if (!isPlainText(path_))
    {
      LOG(debug)
        << "'" << path_ << "' is not a plain text file! Skip saving content.";
    }
    else
      file->content = createFileContent(path_);
  }

  return file;
}

model::FilePtr SourceManager::getFile(const std::string& path_)
{
  //--- Create canonical form of the path ---//

  boost::system::error_code ec;
  boost::filesystem::path canonicalPath
    = boost::filesystem::canonical(path_, ec);

  //--- If the file can't be found on disk then return nullptr ---//

  bool fileExists = true;
  if (ec)
  {
    LOG(debug) << "File doesn't exist: " << path_;
    fileExists = false;
  }

  //--- Create file entry ---//

  std::string canonical = ec ? path_ : canonicalPath.native();

  model::FilePtr file = getCreateFileEntry(canonical, fileExists);

  _createFileMutex.lock();
  _files[canonical] = file;
  _createFileMutex.unlock();

  return file;
}

model::FilePtr SourceManager::getCreateParent(const std::string& path_)
{
  boost::filesystem::path parentPath
    = boost::filesystem::path(path_).parent_path();

  if (parentPath.native().empty())
    return nullptr;

  return getFile(parentPath.native());
}

bool SourceManager::isPlainText(const std::string& path_) const
{
  static std::mutex _magicFileMutex;
  std::lock_guard<std::mutex> guard(_magicFileMutex);

  const char* magic = ::magic_file(_magicCookie, path_.c_str());

  if (!magic)
  {
    LOG(warning) << "Couldn't use magic on file: " << path_;
    return false;
  }

  if (std::strstr(magic, "text"))
    return true;

  return false;
}

void SourceManager::updateFile(const model::File& file_)
{
  _createFileMutex.lock();
  bool find = _persistedFiles.find(file_.id) != _persistedFiles.end();
  _createFileMutex.unlock();

  if (find)
    _transaction([&, this]() {
      _db->update(file_);
    });
}

void SourceManager::persistFiles()
{
  std::lock_guard<std::mutex> guard(_createFileMutex);

  _transaction([&, this]() {
    for (const auto& p : _files)
    {
      if (_persistedFiles.find(p.second->id) == _persistedFiles.end())
        _persistedFiles.insert(p.second->id);
      else
        continue;

      try
      {
        // Directories don't have content.
        if (p.second->content &&
            _persistedContents.find(p.second->content.object_id()) ==
            _persistedContents.end())
        {
          p.second->content.load();
          _db->persist(*p.second->content);
          _persistedContents.insert(p.second->content.object_id());
        }

        _db->persist(*p.second);

        // TODO: The memory consumption should be checked to see if not
        // unloading the lazy shared pointer keeps the file content in memory.
        // If so then this line should be uncommented. The reason for not
        // unloading is that some parsers may want to read the file contents and
        // if this can be done through the File object then the file is not
        // needed to be read from disk.
        p.second->content.unload();
      }
      catch (const odb::object_already_persistent&)
      {
      }
    }
  });
}

} // parser
} // cc
