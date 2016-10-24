#include <iterator>
#include <fstream>
#include <memory>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

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
}

std::vector<std::string> MetricsParser::getDependentParsers() const
{
  return std::vector<std::string>{};
}

bool MetricsParser::parse()
{    
  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    BOOST_LOG_TRIVIAL(info) << "Metrics parse path: " << path;

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
        BOOST_LOG_TRIVIAL(warning)
          << "Metrics parser threw an exception: " << ex_.what();
      }
      catch (...)
      {
        BOOST_LOG_TRIVIAL(warning)
          << "Metrics parser failed with unknown exception!";
      }

    });
  }
  return true;
}

util::DirIterCallback MetricsParser::getParserCallback()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);
    if (boost::filesystem::is_regular_file(path))
    {
      model::FilePtr file = _ctx.srcMgr.getFile(currPath_);
      if(file)
      {
        persistLoc(getLocFromFile(file), file->id);
      }
    }
    return true;
  };
}

MetricsParser::Loc MetricsParser::getLocFromFile(model::FilePtr file_) const
{
  Loc result;

  //--- Get source code ---//

  // TODO: Why doesn't it work? It gives empty string.
  // std::string content = file->content.load()->content;

  BOOST_LOG_TRIVIAL(debug) << "Count metrics for " << file_->path;

  std::ifstream fileStream(file_->path);
  std::string content(
    (std::istreambuf_iterator<char>(fileStream)),
    (std::istreambuf_iterator<char>()));
  fileStream.close();

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
    fileType_ == "CXX" || // Should be updated together with C++ plugin.
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

    metrics.type   = model::Metrics::CODE_LOC;
    metrics.metric = loc_.codeLines;
    _ctx.db->persist(metrics);

    metrics.type   = model::Metrics::NONBLANK_LOC;
    metrics.metric = loc_.nonblankLines;
    _ctx.db->persist(metrics);

    metrics.type   = model::Metrics::ORIGINAL_LOC;
    metrics.metric = loc_.originalLines;
    _ctx.db->persist(metrics);
  });
}

extern "C"
{
  std::shared_ptr<MetricsParser> make(ParserContext& ctx_)
  {
    return std::make_shared<MetricsParser>(ctx_);
  }
}

}
}
