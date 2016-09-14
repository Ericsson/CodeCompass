#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <util/util.h>
#include <util/streamlog.h>
#include <util/filesystem.h>
#include <util/odbtransaction.h>
#include <parser/sourcemanager.h>
#include <model/workspace.h>
#include <model/file-odb.hxx>
#include <model/filecontent.h>
#include <model/filecontent-odb.hxx>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/Twine.h>

namespace
{

using namespace cc;

bool getTimestamp(const std::string& path_, std::uint64_t& timestamp_)
{
  struct stat info;

  if (stat(path_.c_str(), &info) != 0)
  {
    SLog() << "stat() failed for file: '" << path_ << "'";

    return false;
  }

  timestamp_ = info.st_mtim.tv_sec;

  return true;
}

/**
 * Looks at the first and the last 4096 bytes of a file and decides whether
 * it seems to be a text file
 *
 * if the rate of binary characters is more than 15% in the first or last
 * 4096 bytes then it is considered as a binary file
 *
 * @return
 */
bool isTextHeadAndTail(
  const std::string& path_)
{
  typedef std::array<char, 4096> Buffer;

  const double RateLimit = 0.0995851; // based on the work of Rejto" Jeno"

  // Utility function
  auto binaryPercentage = [](const Buffer& buff_, std::streamsize readSize_) {
    int bin = 0;
    for (std::streamsize i = 0; i < readSize_; ++i)
    {
      char c = buff_[i];
      if (static_cast<unsigned char>(buff_[i] < 32) &&
          c != '\n' && c != '\r' && c != '\t')
      {
        ++bin;
      }
    }
    return (double)bin / (double)(readSize_ + 1);
  };

  Buffer buffer;
  std::ifstream file(path_, std::ios::binary);
  if (!file)
  {
    SLog(cc::util::ERROR) << "Error opening file: " << path_;
    return false;
  }

  // Get length of file:
  std::size_t fileSize = 0;
  {
    file.seekg(0, file.end);
    fileSize = file.tellg();
    file.seekg(0, file.beg);
  }

  if (0 == fileSize)
  {
    SLog() << path_ << " is an empty file";
    return false;
  }

  // Test head
  file.read(buffer.data(), buffer.size());
  if (binaryPercentage(buffer, file.gcount()) > RateLimit)
  {
    SLog() << path_ << " is a binary file (head check)";
    return false;
  }

  // Test tail
  if (fileSize > buffer.size())
  {
    file.seekg(
      static_cast<std::ifstream::off_type>(buffer.size()) * -1,
      file.end);
    file.read(buffer.data(), buffer.size());
    if (binaryPercentage(buffer, file.gcount()) > RateLimit)
    {
      SLog() << path_ << " is a binary file (tail check)";
      return false;
    }
  }

  return true;
}

model::File::Type getFileType(const std::string& path_)
{
  std::string ext = util::getPathExtension(path_);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == "cc" || ext == "cpp" || ext == "cxx" || ext == "cp" ||
      ext == "hxx"|| ext == "c++" || ext == "h" || ext == "hh" || ext == "hpp")
    return model::File::CxxSource;

  if (ext == "c")
    return model::File::CSource;

  if (ext == "java")
    return model::File::JavaSource;

  if (ext == "js")
    return model::File::JavaScript;

  if (ext == "pl")
    return model::File::PerlScript;

  if (ext == "sh")
    return model::File::BashScript;
  
  if (ext == "erl")
    return model::File::ErlangSource;
  
  if (ext == "py" || ext == "pyw" || ext == "pyc" || ext == "pyo" || ext == "pyd")
    return model::File::PythonScript;
  
  if (ext == "rb" || ext == "rbw")
    return model::File::RubyScript;
  
  if (ext == "sql")
    return model::File::SqlScript;

  return model::File::GenericFile;
}

bool getPath(const std::string& path_, int flags_, std::string& outPath_)
{
  llvm::SmallString<1024> absPath(path_);
  if (llvm::sys::fs::make_absolute(absPath))
  {
    SLog(util::ERROR) << "make_absolute failed for '" << path_ << "'";
    return false;
  }

  while (absPath.size() > 1 && absPath.back() == '/')
  {
    // Pop last / characters from a path (except for "/" path).
    // Ex1.: /path/a/b/c/    -> /path/a/b/c
    // Ex2.: /path/a/b/c///  -> /path/a/b/c
    absPath.pop_back();
  }

  std::string fullPath =
    (flags_ & parser::SourceManager::SkipPathUnify)
       ? std::string(absPath.str())
       : util::unifyPath(absPath.str());

  // Files with NoContent flag can be virtual
  if ((flags_ & parser::SourceManager::NoContent) == 0 &&
      !llvm::sys::fs::exists(fullPath))
  {
    SLog(util::ERROR) << "'" << path_ << "' does not exists";
    return false;
  }

  outPath_.swap(fullPath);
  return true;
}

} // anonymous namespace

namespace cc
{
namespace parser
{

SourceManager::SourceManager(std::shared_ptr<model::Workspace> ws_, ParseProps& props_) :
  _ws(ws_),
  _props(props_),
  _fileCache(ws_->getDb(), 5741),
  _magicCookie(nullptr)
{
  SLog(util::STATUS) << "Filling file cache...";

  // Load cache
  util::OdbTransaction trans(*ws_->getDb());
  trans([&, this]{
    for (auto file : ws_->getDb()->query<model::File>())
    {
      auto filePtr = std::make_shared<model::File>(file);
      _fileCache.insertToCache(file.path, filePtr);
    }

    for (auto fileContent : ws_->getDb()->query<model::FileContent>())
    {
      _fileContentHashes.insert(fileContent.hash);
    }
  });

  // load magic for plain text testing
  _magicCookie = ::magic_open(MAGIC_SYMLINK);
  if (_magicCookie)
  {
    if (::magic_load(_magicCookie, 0) != 0)
    {
      SLog(util::ERROR) << "magic_load failed! "
         << "libmagic error: " << ::magic_error(_magicCookie);
      
      ::magic_close(_magicCookie);
      _magicCookie = nullptr;
    }
  }
  else
  {
    SLog(util::CRITICAL) << "Failed to create a libmagic cookie!";
  }
}

SourceManager::~SourceManager()
{
  if (_magicCookie)
  {
    ::magic_close(_magicCookie);
  }
}

bool SourceManager::addFile(
  const std::string& path_,
  int flag_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  model::FilePtr file;

  return getCreateFile(path_, file, flag_, buffer_, bufferSize_);
}

bool SourceManager::getCreateFile(
  const std::string& origPath_,
  model::FilePtr& file_,
  int flag_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  // If we fail then we should set the file_ parameter to a null ptr to avoid
  // using a previous result in the calling code (self defense).
  file_.reset();

  std::string path;
  if (!getPath(origPath_, flag_, path))
  {
    return false;
  }

  // -- Try to get an already persisted file #1 --
  {
    std::lock_guard<std::mutex> lock(_createFileMutex);
    file_ = tryGetFile(path);
    if (file_.get())
    {
      return true;
    }
  }

  auto newFile = createFileEntry(path, flag_, buffer_, bufferSize_);
  if (!newFile)
  {
    return false;
  }

#ifndef DATABASE_SQLITE
  // With SQLite we should't lock or otherwise we could stuck in deadlock
  std::lock_guard<std::mutex> lock(_createFileMutex);
#endif

  // FIXME: If the commit fails, then the cache could be inconsistent!
  util::OdbTransaction trans(*_ws->getDb(), true);
  return trans([&, this]() {
#ifdef DATABASE_SQLITE
    // Fuck you SQLite!
    std::lock_guard<std::mutex> lock(_createFileMutex);
#endif

    // -- Try to get an already persisted file #2 --
    file_ = tryGetFile(path);
    if (file_.get())
    {
      return true;
    }

    if (newFile->content)
    {
      if (_fileContentHashes.find(newFile->content->hash) ==
          _fileContentHashes.end())
      {
        SLog(util::DEBUG)
          << "Creating content for file " << path << ". "
          << "Hash = " << newFile->content->hash << " "
          << "Size = " << newFile->content->content.size();

        _ws->getDb()->persist(*newFile->content);
        _fileContentHashes.insert(newFile->content->hash);
      }

      // Performance issue: we only need the file content hash in the File
      // object, so we can drop the actual content and free its allocated
      // memory. If we dont do this we could allocate over 50GB memory only
      // for file contents while parsing some project (eg mmgw).
      newFile->content->content.clear();
      newFile->content->content.shrink_to_fit();
    }

    //SLog() << "Persist file: " << newFile->path;
    auto newFilePtr = newFile.get_eager();
    file_ = _fileCache.getOrInsert(path, newFilePtr);
    return true;
  });
}

bool SourceManager::findFile(
  const std::string& origPath_,
  model::FilePtr& file_,
  int flag_)
{
  // If we fail then we should set the file_ parameter to a null ptr to avoid
  // using a previous result in the calling code (self defense).
  file_.reset();

  std::string path;
  if (!getPath(origPath_, flag_, path))
  {
    return false;
  }

  std::lock_guard<std::mutex> lock(_createFileMutex);
  file_ = tryGetFile(path);
  return file_.get() != nullptr;
}

bool SourceManager::isTextMagic(
  const std::string& path_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  std::unique_lock<std::mutex> lock(_magicCookieMutex);
  
  const char* magic = nullptr;
  if (!buffer_)
  {
    magic = ::magic_file(_magicCookie, path_.c_str());
  }
  else
  {
    magic = ::magic_buffer(_magicCookie, buffer_, bufferSize_);
  }

  if (!magic) {
    SLog(cc::util::ERROR)
      << "Couldn't! use magic on file: " << path_;
    return false;
  }
  
  if (std::strstr(magic, "text")) {
    return true;
  }
  
  if (std::strstr(magic, "XML")) {
    return true;
  }
  
  return false;
}

bool SourceManager::isPlainText(
  const std::string& path_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  // TODO: rewrite isTextHeadAndTail to use buffers
  bool ret = isTextHeadAndTail(path_);
  if (ret && _magicCookie)
  {
    ret = isTextMagic(path_, buffer_, bufferSize_);
  }

  return ret;
}

model::FilePtr SourceManager::tryGetFile(const std::string& path_)
{
  try
  {
    return _fileCache.valueFor(path_);
  }
  catch (const util::ItemNotFound& ex)
  {
    return model::FilePtr();
  }
}

model::FilePtr SourceManager::createFileEntry(
  const std::string& path_,
  int flag_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  // #357: We should not store the "." directories in the database.
  if (path_ == ".")
  {
    return model::FilePtr();
  }

  //SLog() << "Create file entry for " << path_;

  std::uint64_t timestamp = 0;
  getTimestamp(path_, timestamp);

  bool isDirectory = (flag_ & Directory) != 0;

  model::FilePtr fe(new model::File());
  fe->path = path_;
  fe->project = _props.project;
  fe->type = getFileType(path_);
  fe->timestamp = timestamp;
  fe->parent = getParent(path_);

  llvm::sys::fs::file_status status;
  if (!llvm::sys::fs::status(path_, status))
  {
    if (llvm::sys::fs::is_directory(status))
    {
      isDirectory = true;
    }
  }

  if (isDirectory)
  {
    fe->type = model::File::Directory;
  }
  
  fe->filename = util::getPathFilename(path_);
 
  if ((flag_ & NoContent) == 0)
  {
    //
    // Ticket #343: avoid reading directories as files.
    //
    if (!llvm::sys::fs::is_regular_file(status))
    {
      SLog(util::DEBUG)
        << "'" << path_ << "' is not a regular file! Skip saving content.";
    }
    else if (!isPlainText(path_, buffer_, bufferSize_))
    {
      SLog(util::DEBUG)
        << "'" << path_ << "' is not a plain text file! Skip saving content.";
    }
    else
    {
      fe->content = createFileContent(path_, buffer_, bufferSize_);
    }
  }

  fe->id = util::fnvHash(fe->path);
  return fe;
}

model::FileContentPtr SourceManager::createFileContent(
  const std::string& path_,
  const char* buffer_,
  std::size_t bufferSize_)
{
  model::FileContentPtr content = std::make_shared<model::FileContent>();

  //SLog()
  //  << "Create file content for " << path_ << " "
  //  << (buffer_ ? "with buffer" : "without buffer")
  //  << ", buff size = " << bufferSize_;

  if (!buffer_)
  {
    std::ifstream ifs(path_);
    if (!ifs)
    {
      SLog() << "Failed to open '" << path_ << "'";
      return model::FileContentPtr();
    }
    
    // Get length of file:
    ifs.seekg(0, std::ios::end);
    auto fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Get content
    content->content.reserve(fileSize);
    content->content.assign(
      std::istreambuf_iterator<char>(ifs),
      std::istreambuf_iterator<char>());
  }
  else
  {
    content->content.assign(buffer_, bufferSize_);
  }

  // A file may contain 0x00 characters (eg. in an RTF file). If we store
  // these files in a PostgreSql database the we get 'invalid byte
  // sequence' errors.
  // FIXME: convert file content from the file's encoding to the DB's
  //        encoding.
  // FIXME: I'm not sure that SPACE character is the best replacement.
  std::replace(content->content.begin(), content->content.end(), '\0', ' ');

  // Generate hash
  content->hash = util::generateSha1Hash(content->content);
  return content;
}

// Get/add parent folder
model::FilePtr SourceManager::getParent(const std::string& path_)
{
  model::FilePtr parentDir;
  if (!util::isRelativePath(path_) && util::getPathRoot(path_) != path_)
  {
    getCreateFile(util::getPathParent(path_), parentDir,
      NoContent | Directory | SkipPathUnify);
  }
  return parentDir;
}

} //parser
} //cc
