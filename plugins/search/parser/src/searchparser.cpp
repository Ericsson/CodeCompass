#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/Twine.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

#include <indexer/indexerprocess.h>
#include <parser/sourcemanager.h>

#include <searchparser/searchparser.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

std::array<const char*, 16> excludedSuffixes{{
  ".doc", ".rtf", ".htm", ".html", ".xml", ".cc.d", ".cc.opts", ".bin",
  ".hhk", ".hhc", ".output", ".output.0", ".output.1",
  ".Metrics.dat", ".pp", ".tk" // CLAN data files
}};

bool SearchParser::isPlainText(const std::string& path_) const
{
  const char* magic = ::magic_file(_fileMagic, path_.c_str());

  if (!magic)
  {
    BOOST_LOG_TRIVIAL(error) << "Couldn't use magic on file: " << path_;
    return false;
  }

  if (std::strstr(magic, "text"))
    return true;

  return false;
}

SearchParser::SearchParser(ParserContext& ctx_): AbstractParser(ctx_),
  _fileMagic(::magic_open(MAGIC_MIME_TYPE | MAGIC_SYMLINK))
{
  std::string dataDir = ctx_.options["data-dir"].as<std::string>();
  ctx_.options.insert(std::make_pair("search-index-dir",
      po::variable_value(std::string(dataDir + "/search"), false)));

  if (!_fileMagic)
  {
    BOOST_LOG_TRIVIAL(warning) << "Failed to create a libmagic cookie!";
  }
  else
  {
    if (::magic_load(_fileMagic, nullptr) != 0)
    {
      BOOST_LOG_TRIVIAL(warning) << "magic_load failed! libmagic error: "
        << ::magic_error(_fileMagic);

      ::magic_close (_fileMagic);
      _fileMagic = nullptr;
    }
  }

  try
  {
    std::string indexDir = ctx_.options["search-index-dir"].as<std::string>();

    //--- Close last instance (if any) ---//

    _indexProcess.reset();

    //--- Open a new process ---//
    _indexProcess.reset(new IndexerProcess(
      indexDir,
      IndexerProcess::OpenMode::Create));
  }
  catch (const IndexerProcess::Failure& ex_)
  {
    BOOST_LOG_TRIVIAL(error) << "Indexer process failure: " << ex_.what();
  }
}

std::vector<std::string> SearchParser::getDependentParsers() const
{
  return std::vector<std::string>{};
}

bool SearchParser::parse()
{
  for (std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    BOOST_LOG_TRIVIAL(info)<< "Search parse path: " << path;

    util::OdbTransaction trans(_ctx.db);
    trans([&, this]()
      {

        auto cb = getParserCallback(path);
        /*--- Call non-empty iter-callback for all files
         in the current root directory. ---*/
        try
        {
          util::iterateDirectoryRecursive(path, cb);
        }
        catch (std::exception& ex_)
        {
          BOOST_LOG_TRIVIAL(warning)
          << "Search parser threw an exception: "
          << ex_.what();
        }
        catch (...)
        {
          BOOST_LOG_TRIVIAL(warning)
          << "Search parser failed with unknown exception!";
        }
      });
  }
  postParse();

  return true;
}

util::DirIterCallback SearchParser::getParserCallback(const std::string path_)
{
  if (!_indexProcess)
  {
    BOOST_LOG_TRIVIAL(warning) << "Skipping search indexing for path: " << path_;
    return [](const std::string&){ return false; };
  }

  try
  {
    return [this](const std::string& currPath_)
    {
      fs::path path(currPath_);

      if (!fs::is_regular(path))
      {
        return true;
      }

      if (!shouldHandle(currPath_))
      {
        BOOST_LOG_TRIVIAL(info) << "Skipping " << currPath_;

        return true;
      }

      if (!isPlainText(currPath_))
      {
        BOOST_LOG_TRIVIAL(info)
          << "Skipping " << currPath_ << " because it is not plain text.";

        return true;
      }

      model::FilePtr file = _ctx.srcMgr.getCreateFile(currPath_);

      if (file)
      {
        BOOST_LOG_TRIVIAL(info) << "Handling " << currPath_;

        std::string mimeType("text/plain");
        if (_fileMagic)
        {
          const char* mimeStr = ::magic_file(_fileMagic, currPath_.c_str());
          if (mimeStr)
          {
            mimeType = mimeStr;
          }
          else
          {
            BOOST_LOG_TRIVIAL(warning) << "Failed to get mime type for file '"
            << currPath_ << "'. libmagic error: " << ::magic_error(_fileMagic);
          }
        }

        file->inSearchIndex = true;
        _ctx.srcMgr.persistFiles();
        _indexProcess->indexFile(std::to_string(file->id), file->path, mimeType);
      }

      return true;
    };
  }
  catch (const std::exception& ex)
  {
    BOOST_LOG_TRIVIAL(warning)
      <<"Exception thrown in seach parser: " << ex.what();
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
    BOOST_LOG_TRIVIAL(warning) << "stat() failed on " << path_;
    return false;
  }

  //--- The file is larger than one megabyte. ---//

  if (statbuf.st_size > (1024 * 1024))
  {
    return false;
  }

  return true;
}

void SearchParser::postParse()
{
  _indexProcess->buildSuggestions();
  try
  {
    // Wait for indexer process to exit.
    _indexProcess.reset(nullptr);
  }
  catch (...)
  {
    BOOST_LOG_TRIVIAL(error) << "Unknown exception in endTravarse()!";
    throw;
  }
}

SearchParser::~SearchParser()
{
  if (_fileMagic)
  {
    ::magic_close(_fileMagic);
  }
}

extern "C"
{
  std::shared_ptr<SearchParser> make(ParserContext& ctx_)
  {
    return std::shared_ptr<SearchParser>(new SearchParser(ctx_));
  }
}


} // parser
} // cc
