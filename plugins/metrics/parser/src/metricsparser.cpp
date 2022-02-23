#include <iterator>
#include <fstream>
#include <memory>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <parser/sourcemanager.h>

#include <model/metrics.h>
#include <model/metrics-odb.hxx>

#include <metricsparser/metricsparser.h>

namespace cc
{
namespace parser
{

MetricsParser::MetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::MetricsFileIdView& mf
      : _ctx.db->query<model::MetricsFileIdView>())
    {
      _fileIdCache.insert(mf.file);
    }
  });

  int threadNum = _ctx.options["jobs"].as<int>();
  _pool = util::make_thread_pool<std::string>(
    threadNum, [this](const std::string& path_)
    {
      model::FilePtr file = _ctx.srcMgr.getFile(path_);
      if (file)
      {
        if (_fileIdCache.find(file->id) == _fileIdCache.end())
        {
          this->persistLoc(getLocFromFile(file), file->id);
          ++this->_visitedFileCount;
        }
        else
          LOG(debug) << "Metrics already counted for file: " << file->path;
      }
    });
}

bool MetricsParser::cleanupDatabase()
{
  if (!_fileIdCache.empty())
  {
    try
    {
      util::OdbTransaction {_ctx.db} ([this] {
        for (const model::File& file
          : _ctx.db->query<model::File>(
          odb::query<model::File>::id.in_range(_fileIdCache.begin(), _fileIdCache.end())))
        {
          auto it = _ctx.fileStatus.find(file.path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[metricsparser] Database cleanup: " << file.path;

            _ctx.db->erase_query<model::Metrics>(odb::query<model::Metrics>::file == file.id);
            _fileIdCache.erase(file.id);
          }
        }
      });
    }
    catch (odb::database_exception&)
    {
      LOG(fatal) << "Transaction failed in metrics parser!";
      return false;
    }
  }
  return true;
}

bool MetricsParser::parse()
{
  this->_visitedFileCount = 0;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Metrics parse path: " << path;

    util::OdbTransaction trans(_ctx.db);
    trans([&, this]() {
      auto cb = getParserCallback();

      /*--- Call non-empty iter-callback for all files
         in the current root directory. ---*/
      try
      {
        util::iterateDirectoryRecursive(path, cb);
      }
      catch (std::exception& ex_)
      {
        LOG(warning)
          << "Metrics parser threw an exception: " << ex_.what();
      }
      catch (...)
      {
        LOG(warning)
          << "Metrics parser failed with unknown exception!";
      }

    });
  }

  _pool->wait();
  LOG(info) << "Processed files: " << this->_visitedFileCount;

  return true;
}

util::DirIterCallback MetricsParser::getParserCallback()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);

    if (boost::filesystem::is_regular_file(path))
      _pool->enqueue(currPath_);

    return true;
  };
}

MetricsParser::Loc MetricsParser::getLocFromFile(model::FilePtr file_) const
{
  Loc result;

  LOG(debug) << "Count metrics for " << file_->path;

  //--- Get source code ---//

  std::string content
    = file_->content ? file_->content.load()->content : std::string();

  if (content.empty())
    return result;

  //--- Original lines ---//

  result.originalLines = std::count(content.begin(), content.end(), '\n') + 1;

  //--- Non blank lines ---//

  eraseBlankLines(content);

  result.nonblankLines = std::count(content.begin(), content.end(), '\n') + 1;

  //--- Code lines ---//

  std::string singleComment, multiCommentStart, multiCommentEnd;

  setCommentTypes(
    file_->type, singleComment, multiCommentStart, multiCommentEnd);
  eraseComments(
    content, singleComment, multiCommentStart, multiCommentEnd);

  result.codeLines = std::count(content.begin(), content.end(), '\n') + 1;

  return result;
}

void MetricsParser::setCommentTypes(
  std::string& fileType_,
  std::string& singleComment_,
  std::string& multiCommentStart_,
  std::string& multiCommentEnd_) const
{
  if (
    fileType_ == "CPP" || // Should be updated together with C++ plugin.
    fileType_ == "Java")
  {
    singleComment_ = "//";
    multiCommentStart_ = "/*";
    multiCommentEnd_ = "*/";
  }
  else if (
    fileType_ == "Erlang" ||
    fileType_ == "Bash" ||
    fileType_ == "Perl")
  {
    singleComment_ = "#";
    multiCommentStart_ = "";  //multi line comment not exist
    multiCommentEnd_ = "";
  }
  else if (fileType_ == "Python")
  {
    singleComment_ = "#";
    multiCommentStart_ = R"(""")";
    multiCommentEnd_ = R"(""")";
  }
  else if (fileType_ == "Sql")
  {
    singleComment_ = "--";
    multiCommentStart_ = "/*";
    multiCommentEnd_ = "*/";
  }
  else if (fileType_ == "Ruby")
  {
    singleComment_ = "#";
    multiCommentStart_ = "=begin";
    multiCommentEnd_ = "\n=end";  //end must be at new line
  }
  else //default value
  {
    singleComment_ = "";
    multiCommentStart_ = "";
    multiCommentEnd_ = "";
  }
}

void MetricsParser::eraseBlankLines(std::string& file_) const
{
  std::string::iterator first = file_.begin();
  bool isBlankLine = true;
  for (std::string::iterator it = first; it != file_.end(); ++it)
  {
    if (!std::isspace(*it))
    {
      isBlankLine = false;
    }
    else if (*it == '\n')
    {
      if (isBlankLine && first != it)
      {
        it = file_.erase(first, it);
      }
      first = it + 1;
      isBlankLine = true;
    }
  }

  file_.erase(std::unique(file_.begin(), file_.end(),
    [](char a, char b) { return a == '\n' && b == '\n'; }), file_.end());
}

void MetricsParser::eraseComments(
  std::string& file_,
  const std::string& singleComment_,
  const std::string& multiCommentStart_,
  const std::string& multiCommentEnd_) const
{
  const int s = multiCommentStart_.size();

  // Simple line
  if (!singleComment_.empty()) // singleComment exist
  {
    std::size_t start = file_.find(singleComment_);
    for (std::size_t end = file_.find('\n', start);
         start != std::string::npos;
         start = file_.find(singleComment_), end = file_.find('\n', start))
    {
      if (end == std::string::npos) // last line case
      {
        end = file_.size();
      }
      file_.erase(start, end - start);
    }
  }

  // Multiline
  if (!multiCommentStart_.empty()) // multiComment exist
  {
    for (std::size_t start = file_.find(multiCommentStart_),
                     end   = file_.find(multiCommentEnd_, start + s);
         start != std::string::npos;
         start = file_.find(multiCommentStart_),
         end   = file_.find(multiCommentEnd_, start + s))
    {
      // Delete end comment symbol too
      file_.erase(start, end - start + s);
    }
  }
}

void MetricsParser::persistLoc(const Loc& loc_, model::FileId file_)
{
  util::OdbTransaction trans(_ctx.db);
  trans([&, this]{
    model::Metrics metrics;
    metrics.file = file_;

    if (loc_.codeLines != 0)
    {
      metrics.type   = model::Metrics::CODE_LOC;
      metrics.metric = loc_.codeLines;
      _ctx.db->persist(metrics);
    }

    if (loc_.nonblankLines != 0)
    {
      metrics.type   = model::Metrics::NONBLANK_LOC;
      metrics.metric = loc_.nonblankLines;
      _ctx.db->persist(metrics);
    }

    if (loc_.originalLines != 0)
    {
      metrics.type   = model::Metrics::ORIGINAL_LOC;
      metrics.metric = loc_.originalLines;
      _ctx.db->persist(metrics);
    }
  });
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Metrics Plugin");
    return description;
  }

  std::shared_ptr<MetricsParser> make(ParserContext& ctx_)
  {
    return std::make_shared<MetricsParser>(ctx_);
  }
}
#pragma clang diagnostic pop

}
}
