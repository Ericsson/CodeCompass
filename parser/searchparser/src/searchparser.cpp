#include "searchparser/searchparser.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <array>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/ADT/Twine.h"

#include <model/workspace.h>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/util.h>
#include <util/streamlog.h>

#include <indexer-api/indexerprocess.h>
#include <parser/sourcemanager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cc
{
namespace parser
{

std::array<const char*, 16> excludedSuffixes{{
  ".doc", ".rtf", ".htm", ".html", ".xml", ".cc.d", ".cc.opts", ".bin",
  ".hhk", ".hhc", ".output", ".output.0", ".output.1",
  ".Metrics.dat", ".pp", ".tk" // CLAN data files
}};

SearchParser::SearchParser(std::shared_ptr<model::Workspace> w_) :
  _workspace(w_),
  _fileMagic(::magic_open(MAGIC_MIME_TYPE | MAGIC_SYMLINK))
{
  if (!_fileMagic)
  {
    SLog(util::CRITICAL) << "Failed to create a libmagic cookie!";
  }
  else
  {
    if (::magic_load(_fileMagic, nullptr) != 0)
    {
      SLog(util::ERROR) << "magic_load failed! "
         << "libmagic error: " << ::magic_error(_fileMagic);

      ::magic_close(_fileMagic);
      _fileMagic = nullptr;
    }
  }
}

SearchParser::~SearchParser()
{
  if (_fileMagic)
  {
    ::magic_close(_fileMagic);
  }
}

void SearchParser::beforeTraverse(
  const Traversal::OptionMap& projectOptions_,
  SourceManager&)
{
  try
  {
    std::string indexDir = projectOptions_.at("searchIndexDir");

    // close last instance (if any)
    _indexProcess.reset();

    // open a new process
    _indexProcess.reset(new IndexerProcess(
      indexDir,
      IndexerProcess::OpenMode::Create));
  }
  catch (const IndexerProcess::Failure& ex_)
  {
    SLog(util::ERROR) << "Indexer process failure: " << ex_.what();
  }
}

void SearchParser::afterTraverse(SourceManager&)
{
  try
  {
    // Wait for indexer process to exit.
    _indexProcess.reset(nullptr);
  }
  catch (...)
  {
    SLog(util::CRITICAL) << "Unknown exception in endTravarse()!";
    throw;
  }
}

Traversal::DirIterCallback SearchParser::traverse(
  const std::string& path_,
  SourceManager& srcMgr_)
{
  if (!_indexProcess)
  {
    SLog(util::CRITICAL) << "Skipping search indexing for path: " << path_;
    return Traversal::DirIterCallback();
  }

  try
  {
    return [this, &srcMgr_](
      const std::string& currPath, Traversal::FileType currType)
    {
      if (currType != Traversal::FileType::RegularFile)
      {
        return true;
      }

      if (!shouldHandle(currPath))
      {
        SLog() << "Skipping " << currPath;

        return true;
      }

      if (!srcMgr_.isPlainText(currPath))
      {
        SLog() << "Skipping " << currPath << " because it is not plain text.";

        return true;
      }

      model::FilePtr file;
      if (srcMgr_.getCreateFile(currPath, file))
      {
        SLog() << "Handling " << currPath;

        std::string mimeType("text/plain");
        if (_fileMagic)
        {
          const char* mimeStr = ::magic_file(_fileMagic, currPath.c_str());
          if (mimeStr)
          {
            mimeType = mimeStr;
          }
          else
          {
            SLog(util::ERROR) << "Failed to get mime type for file '"
              << currPath << "'. libmagic error: " << ::magic_error(_fileMagic);
          }
        }

        file->inSearchIndex = true;
        _workspace->getDb()->update(file.get_eager());

        _indexProcess->indexFile(std::to_string(file->id),file->path,mimeType);
      }

      return true;
    };
  }
  catch (const std::exception& ex)
  {
    SLog(util::CRITICAL) <<
      "Exception thrown in SeachParser::traverse: " << ex.what();
    throw;
  }
}

bool SearchParser::shouldHandle(const std::string& path_)
{
  std::string normPath(path_);
  std::transform(normPath.begin(), normPath.end(), normPath.begin(), ::tolower);

  for (const char* suff : excludedSuffixes)
  {
    std::size_t sufflen = ::strlen(suff);

    if (normPath.length() >= sufflen &&
        normPath.compare(normPath.length() - sufflen, sufflen, suff) == 0)
    {
      return false;
    }
  }

  struct stat statbuf;
  if (::stat(path_.c_str(), &statbuf) == -1)
  {
    SLog(util::WARNING) << "stat() failed on " << path_;
    return false;
  }

  if (statbuf.st_size > (1024 * 1024))
  {
    // The file is larger than one megabyte.
    return false;
  }

  return true;
}

} // parser
} // cc
