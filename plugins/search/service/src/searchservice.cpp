#include <limits>
#include <cctype>
#include <memory>
#include <ctime>
#include <chrono>

#include <boost/filesystem.hpp>

#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/query.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>

#include <service/searchservice.h>

namespace fs = boost::filesystem;

namespace
{

using cc::service::search::SearchServiceHandler;

class FilterHelper
{
public:
  FilterHelper(const cc::service::search::SearchFilter& filters_) :
    _filters(filters_),
    _fileFilter(_filters.fileFilter, boost::regex::icase),
    _dirFilter(_filters.dirFilter, boost::regex::icase)
  {
  }

public:
  bool shouldSkip(const std::string& filePath_) const
  {
    fs::path path(filePath_);
    std::string filename = path.filename().native();

    bool skip = false;
    if (!skip && !_filters.fileFilter.empty())
    {
      skip = shouldSkipByFilter(_fileFilter, filename);
    }

    if (!skip && !_filters.dirFilter.empty())
    {
      skip = shouldSkipByFilter(_dirFilter, filename);
    }

    return skip;
  }

private:
  /**
   * This method does the filtering.
   *
   * @param filter_ the filter regular expression.
   * @param filePath_ a file path.
   * @return returns true if we should skip this file according to the filter
   *         expression.
   */
  static bool shouldSkipByFilter(
      const boost::regex&     filter_,
      const std::string&      filePath_)
  {
    try
    {
      return !boost::regex_search(filePath_, filter_);
    }
    catch (const boost::regex_error& err)
    {
      LOG(warning)
        << "Search service threw an exception: " << err.what();

      return false;
    }

    return false;
  }

private:
  const cc::service::search::SearchFilter& _filters;
  boost::regex _fileFilter;
  boost::regex _dirFilter;
};

} // anonymous namespace

namespace cc
{
namespace service
{
namespace search
{

SearchServiceHandler::SearchServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_) :
    _db(db_)
{
  _javaProcess.reset(new ServiceProcess(*datadir_ + "/search",
                                        context_.compassRoot,
                                        context_.options.count("log-target")
                                          ? context_.options["log-target"].as<std::string>()
                                          : ""));
}

void SearchServiceHandler::search(
  SearchResult& _return,
  const SearchParams& params_)
{
  std::lock_guard<std::mutex> lock(_javaProcessMutex);

  try
  {
    auto start = std::chrono::steady_clock::now();

    _javaProcess->search(_return, params_);

    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    LOG(info) << "Search time: " << dur.count() << " milliseconds.";
  }
  catch (const ServiceProcess::ProcessDied&)
  {
    LOG(error) << "Java search service died! Terminating server...";
    ::abort();
  }
}

void SearchServiceHandler::searchFile(
    FileSearchResult& _return,
    const SearchParams&     params_)
{
  LOG(info) << "Search for file: query = " << params_.query;

  odb::transaction t(_db->begin());

  typedef odb::result<model::ParentIdCollector> parentIds;
  typedef odb::result<model::File> fileResult;
  typedef odb::query<model::File> query;

  validateRegexp(params_.query);

  try
  {
    FilterHelper filters(params_.filter);

    parentIds parents = _db->query<model::ParentIdCollector>(
      query::type != model::File::DIRECTORY_TYPE &&
      query::filename + SQL_REGEX + query::_val(params_.query));

    std::vector<model::FileId> parentVector;
    for (const auto& parent : parents)
    {
      parentVector.push_back(parent.parent);
    }

    std::size_t minIdx = 0;
    std::size_t maxIdx = parentVector.size();
    if (params_.__isset.range)
    {
      minIdx = std::min(static_cast<int64_t>(maxIdx), params_.range.start);
      maxIdx = std::min(static_cast<int64_t>(maxIdx),
        params_.range.start + params_.range.maxSize);
    }

    if (minIdx == maxIdx)
    {
      // No result
      _return.totalFiles = 0;
      return;
    }

    fileResult r (_db->query<model::File>(
      query::type != model::File::DIRECTORY_TYPE &&
      query::filename + SQL_REGEX + query::_val(params_.query) &&
      query::parent.in_range(parentVector.begin() + minIdx,
        parentVector.begin() + maxIdx)));

    for (const auto& f : r)
    {
      if (filters.shouldSkip(f.path))
      {
        continue;
      }

      core::FileInfo info;
      info.id = std::to_string(f.id);
      info.name = f.filename;
      info.path = f.path;

      _return.results.push_back(info);
      _return.totalFiles = parentVector.size();
    }
  }
  catch (odb::exception &odbex)
  {
    LOG(error)
      << "Search service search in file exception: " << odbex.what();

    DatasourceError ex;
    ex.message = odbex.what();
    throw ex;
  }
  catch (const boost::regex_error& err)
  {
    LOG(error) << "Regexp error: " << err.what();

    SearchException ex;
    ex.message  = "Bad regular expression: ";
    ex.message += err.what();
    throw ex;
  }
}


void SearchServiceHandler::getSearchTypes(
    std::vector<SearchType> & _return)
{
  struct {
    const char* name;
    uint32_t option;
  } options[] = {
    { "Text search",
      ::cc::service::search::SearchOptions::SearchInSource },
    { "Definition search",
      ::cc::service::search::SearchOptions::SearchInDefs },
    { "File name search",
      ::cc::service::search::SearchOptions::SearchForFileName },
    { "Log search",
      ::cc::service::search::SearchOptions::FindLogText }
  };

  for (auto t : options)
  {
    ::cc::service::search::SearchType type;

    type.id = t.option;
    type.name = t.name;

    _return.push_back(type);
  }
}

void SearchServiceHandler::pleaseStop()
{
}

void SearchServiceHandler::suggest(SearchSuggestions& _return,
  const SearchSuggestionParams& params_)
{
  std::lock_guard<std::mutex> lock(_javaProcessMutex);

  try
  {
    auto start = std::chrono::steady_clock::now();

    _javaProcess->suggest(_return, params_);

    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    LOG(info) << "Suggest time: " << dur.count() << " milliseconds.";
  }
  catch (const ServiceProcess::ProcessDied&)
  {
    LOG(error) << "Java search service died! Terminating server...";
    ::abort();
  }
}

void SearchServiceHandler::validateRegexp(const std::string& regexp_)
{
  try
  {
    boost::regex regex(regexp_);
  }
  catch (const boost::regex_error& err)
  {
    SearchException ex;
    ex.message  = "Bad regular expression: ";
    ex.message += err.what();
    throw ex;
  }
}

} // search
} // service
} // cc

