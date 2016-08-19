#include <fstream>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <parser/sourcemanager.h>
#include <util/odbtransaction.h>
#include <util/hash.h>

namespace
{

cc::model::File::Type getFileType(const std::string& path_)
{
  std::string ext = boost::filesystem::path(path_).extension().native();
  std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

  if (ext == ".cc"  || ext == ".cpp" || ext == ".cxx" || ext == ".cp" ||
      ext == ".hxx" || ext == ".c++" || ext == ".h"   || ext == ".hh" ||
      ext == ".hpp")
    return cc::model::File::CxxSource;

  if (ext == ".c")
    return cc::model::File::CSource;

  if (ext == ".java")
    return cc::model::File::JavaSource;

  if (ext == ".js")
    return cc::model::File::JavaScript;

  if (ext == ".pl")
    return cc::model::File::PerlScript;

  if (ext == ".sh")
    return cc::model::File::BashScript;

  if (ext == ".erl")
    return cc::model::File::ErlangSource;

  if (ext == ".py" || ext == ".pyw" || ext == ".pyc" || ext == ".pyc" ||
      ext == ".pyd")
    return cc::model::File::PythonScript;

  if (ext == ".rb" || ext == ".rbw")
    return cc::model::File::RubyScript;

  if (ext == ".sql")
    return cc::model::File::SqlScript;

  return cc::model::File::GenericFile;
}

}

namespace cc
{
namespace parser
{

SourceManager::SourceManager(std::shared_ptr<odb::database> db_)
  : _db(db_)//, _magicCookie(nullptr)
{
  //--- Reload files from database ---//

  util::OdbTransaction trans(_db);
  trans([&, this]() {
    for (const model::File& file : db_->query<model::File>())
    {
      _files[file.path] = std::make_shared<model::File>(file);
      _persistedFiles.insert(file.id);
    }
  });

  //--- Initialize magic for plain text testing ---//

//  if (_magicCookie = ::magic_open(MAGIC_SYMLINK))
//  {
//    if (::magic_load(_magicCookie, 0) != 0)
//    {
//      BOOST_LOG_TRIVIAL(error)
//        << "libmagic error: "
//        << ::magic_error(_magicCookie);
//
//      ::magic_close(_magicCookie);
//      _magicCookie = nullptr;
//    }
//  }
//  else
//    BOOST_LOG_TRIVIAL(error) << "Failed to create a libmagic cookie!";
}

SourceManager::~SourceManager()
{
  //persistFiles();

//  if (_magicCookie)
//    ::magic_close(_magicCookie);
}

model::FilePtr SourceManager::getFile(const std::string& path_)
{
  std::lock_guard<std::mutex> guard(_createFileMutex);
  return getCreateFile(path_);
}

model::FileContentPtr SourceManager::createFileContent(
  const std::string& path_) const
{
  std::ifstream ifs(path_);
  if (!ifs)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to open '" << path_ << "'";
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

  std::unordered_map<std::string, model::FilePtr>::const_iterator it
    = _files.find(path_);

  if (it != _files.end())
    return it->second;

  //--- Create new file entry ---//

  boost::system::error_code ec;
  boost::filesystem::path path(path_);

  std::time_t timestamp = boost::filesystem::last_write_time(path, ec);
  if (ec)
    timestamp = 0;

  model::FilePtr file(new model::File());
  file->id = util::fnvHash(path_);
  file->path = path_;
  file->type = getFileType(path_);
  file->timestamp = timestamp;
  file->parent = getCreateParent(path_);
  file->filename = path.filename().native();

  if (boost::filesystem::is_directory(path, ec))
    file->type = model::File::Directory;

  if (file->type != model::File::Directory && withContent_)
  {
    if (!boost::filesystem::is_regular_file(path, ec))
    {
      BOOST_LOG_TRIVIAL(debug)
        << "'" << path_ << "' is not a regular file! Skip saving content.";
    }
    else if (!isPlainText(path_))
    {
      BOOST_LOG_TRIVIAL(debug)
        << "'" << path_ << "' is not a plain text file! Skip saving content.";
    }
    else
      file->content = createFileContent(path_);
  }

  return file;
}

model::FilePtr SourceManager::getCreateFile(const std::string& path_)
{
  //--- Create canonical form of the path ---//

  boost::system::error_code ec;
  boost::filesystem::path canonicalPath
    = boost::filesystem::canonical(path_, ec);

  //--- If the file can't be found on disk then return nullptr ---//

  if (ec)
  {
    BOOST_LOG_TRIVIAL(debug) << "File doesn't exist: " << path_;
    return model::FilePtr();
  }

  //--- Create file entry ---//

  std::string canonical = canonicalPath.native();
  return _files[canonical] = getCreateFileEntry(canonical);
}

model::FilePtr SourceManager::getCreateParent(const std::string& path_)
{
  boost::filesystem::path parentPath
    = boost::filesystem::path(path_).parent_path();

  if (parentPath.native().empty())
    return nullptr;

  return getCreateFile(parentPath.native());
}

bool SourceManager::isPlainText(const std::string& path_) const
{
//  const char* magic = ::magic_file(_magicCookie, path_.c_str());
//
//  if (!magic)
//  {
//    BOOST_LOG_TRIVIAL(error) << "Couldn't use magic on file: " << path_;
//    return false;
//  }
//
//  if (std::strstr(magic, "text"))
//    return true;
//
//  return false;
  return true;
}

void SourceManager::persistFiles()
{
  std::lock_guard<std::mutex> guard(_createFileMutex);

  util::OdbTransaction trans(_db);
  trans([&, this]() {
    for (const auto& p : _files)
    {
      if (_persistedFiles.find(p.second->id) == _persistedFiles.end())
        _persistedFiles.insert(p.second->id);
      else
        continue;

      try
      {
        // Directories don't have content.
        if (p.second->content)
        {
          p.second->content.load();
          _db->persist(*p.second->content);
        }

        _db->persist(*p.second);

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
