#include <string>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <parser/sourcemanager.h>
#include <indexer/indexerprocess.h>
#include <searchparser/searchparser.h>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

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
    LOG(warning) << "Failed to create a libmagic cookie!";
  }
  else if (::magic_load(_fileMagic, nullptr) != 0)
  {
    LOG(warning)
      << "magic_load failed! libmagic error: "
      << ::magic_error(_fileMagic);

    ::magic_close(_fileMagic);
    _fileMagic = nullptr;
  }

  std::string wsDir = ctx_.options["workspace"].as<std::string>();
  std::string projDir = wsDir + '/' + ctx_.options["name"].as<std::string>();
  _searchDatabase = projDir + "/search";

  if (_ctx.options.count("search-skip-directory"))
    for (const std::string& path
      : _ctx.options["search-skip-directory"].as<std::vector<std::string>>())
    {
      _skipDirectories.push_back(fs::canonical(fs::absolute(path)).string());
    }

  try
  {
    //--- Close last instance (if any) ---//

    _indexProcess.reset();

    //--- Open a new process ---//

    _indexProcess.reset(new IndexerProcess(
      _searchDatabase,
      ctx_.compassRoot,
      IndexerProcess::OpenMode::Create,
      IndexerProcess::LockMode::Simple,
      ctx_.options.count("logtarget")
        ? ctx_.options["logtarget"].as<std::string>()
        : ""));
  }
  catch (const IndexerProcess::Failure& ex_)
  {
    LOG(error) << "Indexer process failure: " << ex_.what();
  }
}

bool SearchParser::parse()
{
  if (fs::is_directory(_searchDatabase))
  {
    fs::remove_all(_searchDatabase);
    fs::create_directory(_searchDatabase);
    LOG(info) << "Search database already exists, dropping.";
  }

  for (const std::string& path :
    _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Search parse path: " << path;

    try
    {
      util::iterateDirectoryRecursive(path, getParserCallback(path));
    }
    catch (const std::exception& ex_)
    {
      LOG(warning) << "Search parser threw an exception: " << ex_.what();
    }
    catch (...)
    {
      LOG(warning) << "Search parser failed with unknown exception!";
    }
  }

  postParse();

  return true;
}

util::DirIterCallback SearchParser::getParserCallback(const std::string& path_)
{
  if (!_indexProcess)
  {
    LOG(warning) << "Indexer process is not available, skip path: " << path_;
    return [](const std::string&){ return false; };
  }

  return [this](const std::string& currPath_)
  {
    if (fs::is_directory(currPath_))
    {
      fs::path canonicalPath = fs::canonical(currPath_);

      if (std::find(_skipDirectories.begin(), _skipDirectories.end(),
            canonicalPath) != _skipDirectories.end())
      {
        LOG(trace) << "Skipping " << currPath_ << " because it was listed in "
          "the skipping directory flag of the search parser.";
        return false;
      }
    }

    if (!shouldHandle(currPath_))
      return true;

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
          LOG(warning)
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
  //--- The file is not regular. ---//

  if (!fs::is_regular_file(path_))
    return false;

  //--- The file is excluded by suffix. ---//

  std::string normPath(path_);
  std::transform(normPath.begin(), normPath.end(), normPath.begin(), ::tolower);

  for (const char* suff : excludedSuffixes)
  {
    std::size_t sufflen = ::strlen(suff);

    if (normPath.length() >= sufflen &&
        normPath.compare(normPath.length() - sufflen, sufflen, suff) == 0)
    {
      LOG(trace) << "Skipping " << path_;
      return false;
    }
  }

  //--- The file is larger than one megabyte. ---//

  struct stat statbuf;
  if (::stat(path_.c_str(), &statbuf) == -1)
    return false;

  if (statbuf.st_size > (1024 * 1024))
    return false;

  //--- The file is not plain text. ---//

  if (!_ctx.srcMgr.isPlainText(path_))
  {
    LOG(trace) << "Skipping " << path_ << " because it is not plain text.";
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
    LOG(warning) << "Unknown exception in endTravarse()!";
  }
}

SearchParser::~SearchParser()
{
  if (_fileMagic)
    ::magic_close(_fileMagic);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Search Plugin");

    description.add_options()
      ("search-skip-directory", po::value<std::vector<std::string>>(),
       "Directories can be skipped during the parse. Here you can list the "
       "paths of the directories.");

    return description;
  }

  std::shared_ptr<SearchParser> make(ParserContext& ctx_)
  {
    return std::shared_ptr<SearchParser>(new SearchParser(ctx_));
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
