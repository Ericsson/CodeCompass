#include <string>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <parser/sourcemanager.h>
#include <indexer/indexerprocess.h>
#include <searchparser/searchparser.h>

namespace cc
{
namespace parser
{

// TODO: These should come from command line arguments.
std::array<const char*, 15> excludedSuffixes{{
  ".doc", ".rtf", ".htm", ".html", ".xml", ".cc.d", ".cc.opts", ".bin",
  ".hhk", ".hhc", ".output", ".output.0", ".output.1",
  ".Metrics.dat", ".pp"
}};

SearchParser::SearchParser(ParserContext& ctx_) : AbstractParser(ctx_),
  _fileMagic(::magic_open(MAGIC_MIME_TYPE | MAGIC_SYMLINK))
{
  if (!_fileMagic)
  {
    BOOST_LOG_TRIVIAL(warning) << "Failed to create a libmagic cookie!";
  }
  else if (::magic_load(_fileMagic, nullptr) != 0)
  {
    BOOST_LOG_TRIVIAL(warning)
      << "magic_load failed! libmagic error: "
      << ::magic_error(_fileMagic);

    ::magic_close(_fileMagic);
    _fileMagic = nullptr;
  }

  try
  {
    std::string indexDir
      = ctx_.options["data-dir"].as<std::string>() + "/search";

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
  for (const std::string& path :
    _ctx.options["input"].as<std::vector<std::string>>())
  {
    BOOST_LOG_TRIVIAL(info) << "Search parse path: " << path;

    try
    {
      util::iterateDirectoryRecursive(path, getParserCallback(path));
    }
    catch (const std::exception& ex_)
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
  }

  postParse();

  return true;
}

util::DirIterCallback SearchParser::getParserCallback(const std::string& path_)
{
  if (!_indexProcess)
  {
    BOOST_LOG_TRIVIAL(warning)
      << "Indexer process is not available, skip path: " << path_;
    return [](const std::string&){ return false; };
  }

  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);

    if (!boost::filesystem::is_regular(path))
      return true;

    if (!shouldHandle(currPath_))
    {
      BOOST_LOG_TRIVIAL(info) << "Skipping " << currPath_;
      return true;
    }

    if (!_ctx.srcMgr.isPlainText(currPath_))
    {
      BOOST_LOG_TRIVIAL(info)
        << "Skipping " << currPath_ << " because it is not plain text.";
      return true;
    }

    model::FilePtr file = _ctx.srcMgr.getFile(currPath_);

    if (file)
    {
      std::string mimeType("text/plain");
      if (_fileMagic)
      {
        const char* mimeStr = ::magic_file(_fileMagic, currPath_.c_str());

        if (mimeStr)
          mimeType = mimeStr;
        else
          BOOST_LOG_TRIVIAL(warning)
            << "Failed to get mime type for file '"
            << currPath_ << "'. libmagic error: "
            << ::magic_error(_fileMagic);
      }

      file->inSearchIndex = true;
      _ctx.srcMgr.persistFiles();
      _indexProcess->indexFile(
        std::to_string(file->id), file->path, mimeType);
    }

    return true;
  };
}

bool SearchParser::shouldHandle(const std::string& path_)
{
  //--- The file is excluded by suffix. ---//

  std::string normPath(path_);
  std::transform(normPath.begin(), normPath.end(), normPath.begin(), ::tolower);

  for (const char* suff : excludedSuffixes)
  {
    std::size_t sufflen = ::strlen(suff);

    if (normPath.length() >= sufflen &&
        normPath.compare(normPath.length() - sufflen, sufflen, suff) == 0)
      return false;
  }

  //--- The file is larger than one megabyte. ---//

  struct stat statbuf;
  if (::stat(path_.c_str(), &statbuf) == -1)
    return false;

  if (statbuf.st_size > (1024 * 1024))
    return false;

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
    BOOST_LOG_TRIVIAL(warning) << "Unknown exception in endTravarse()!";
  }
}

SearchParser::~SearchParser()
{
  if (_fileMagic)
    ::magic_close(_fileMagic);
}

extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Search Plugin");
    return description;
  }

  std::shared_ptr<SearchParser> make(ParserContext& ctx_)
  {
    return std::shared_ptr<SearchParser>(new SearchParser(ctx_));
  }
}


} // parser
} // cc
