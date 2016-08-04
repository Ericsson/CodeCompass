#include <iterator>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <util/db/dbutil.h>
#include <util/db/odbtransaction.h>

#include <parser/sourcemanager.h>

#include <model/metrics.h>
#include <model/metrics-odb.hxx>

#include <metricsparser/metricsparser.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

MetricsParser::MetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _db = cc::util::createDatabase(_ctx.options["database"].as<std::string>());
}
  
std::string MetricsParser::getName() const
{
  return "metricsparser";
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


      util::OdbTransaction trans(_db);
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
  return [this](
    const std::string& currPath_)
  {
    fs::path path(currPath_);
    if (fs::is_regular_file(path))
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

MetricsParser::Loc MetricsParser::getLocFromFile(model::FilePtr file) const
{
  Loc result;

  //--- Get source code ---//

  // TODO: Why doesn't it work? It gives empty string.
  // std::string content = file->content.load()->content;

  BOOST_LOG_TRIVIAL(debug) << "Count LOC metrics for " << file->path;

  std::ifstream fileStream(file->path);
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
    file->type, singleComment, multiCommentStart, multiCommentEnd);
  eraseComments(
    content, singleComment, multiCommentStart, multiCommentEnd);

  result.codeLines = std::count(content.begin(), content.end(), '\n') + 1;

  return result;
}

void MetricsParser::setCommentTypes(
  model::File::Type& type,
  std::string& singleComment,
  std::string& multiCommentStart,
  std::string& multiCommentEnd) const
{
  if (
    type == model::File::CSource ||
    type == model::File::CxxSource ||
    type == model::File::JavaSource)
  {
    singleComment = "//";
    multiCommentStart = "/*";
    multiCommentEnd = "*/";
  }
  else if (
    type == model::File::ErlangSource ||
    type == model::File::BashScript ||
    type == model::File::PerlScript)
  {
    singleComment = "#";
    multiCommentStart = "";  //multi line comment not exist
    multiCommentEnd = "";
  }
  else if (type == model::File::PythonScript)
  {
    singleComment = "#";
    multiCommentStart = R"(""")";
    multiCommentEnd = R"(""")";
  }
  else if (type == model::File::SqlScript)
  {
    singleComment = "--";
    multiCommentStart = "/*";
    multiCommentEnd = "*/";
  }
  else if (type == model::File::RubyScript)
  {
    singleComment = "#";
    multiCommentStart = "=begin";
    multiCommentEnd = "\n=end";  //end must be at new line
  }
  else //default value
  {
    singleComment = "";
    multiCommentStart = "";
    multiCommentEnd = "";
  }
}

void MetricsParser::eraseBlankLines(std::string& file) const
{
  std::string::iterator first = file.begin();
  bool isBlankLine = true;
  for (std::string::iterator it = first; it != file.end(); ++it)
  {
    if (!std::isspace(*it))
    {
      isBlankLine = false;
    }
    else if (*it == '\n')
    {
      if (isBlankLine && first != it)
      {
        it = file.erase(first, it);
      }
      first = it + 1;
      isBlankLine = true;
    }
  }

  file.erase(std::unique(file.begin(), file.end(),
    [](char a, char b) { return a == '\n' && b == '\n'; }), file.end());
}

void MetricsParser::eraseComments(
  std::string& file,
  const std::string& singleComment,
  const std::string& multiCommentStart,
  const std::string& multiCommentEnd) const
{
  const int s = multiCommentStart.size();

  // Simple line
  if (!singleComment.empty()) // singleComment exist
  {
    std::size_t start = file.find(singleComment);
    for (std::size_t end = file.find('\n', start);
         start != std::string::npos;
         start = file.find(singleComment), end = file.find('\n', start))
    {
      if (end == std::string::npos) // last line case
      {
        end = file.size();
      }
      file.erase(start, end - start);
    }
  }

  // Multiline
  if (!multiCommentStart.empty()) // multiComment exist
  {
    for (std::size_t start = file.find(multiCommentStart),
                     end   = file.find(multiCommentEnd, start + s);
         start != std::string::npos;
         start = file.find(multiCommentStart),
         end   = file.find(multiCommentEnd, start + s))
    {
      // Delete end comment symbol too
      file.erase(start, end - start + multiCommentStart.size());
    }
  }

  eraseBlankLines(file);
}

void MetricsParser::persistLoc(const Loc& loc, model::FileId file)
{
  util::OdbTransaction trans(_db);
  trans([&, this]{
    model::Metrics metrics;
    metrics.file = file;

    metrics.type   = model::Metrics::CODE_LOC;
    metrics.metric = loc.codeLines;
    _db->persist(metrics);

    metrics.type   = model::Metrics::NONBLANK_LOC;
    metrics.metric = loc.nonblankLines;
    _db->persist(metrics);

    metrics.type   = model::Metrics::ORIGINAL_LOC;
    metrics.metric = loc.originalLines;
    _db->persist(metrics);
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
