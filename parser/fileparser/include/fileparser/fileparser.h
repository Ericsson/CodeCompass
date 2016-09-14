#ifndef FILEPARSER_FILEPARSER_H
#define FILEPARSER_FILEPARSER_H

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <atomic>
#include <future>

#include <model/buildaction.h>
#include <parser/commondefs.h>

namespace cc
{
namespace parser
{

namespace search
{

class IndexerServiceIf;

}

class SourceManager;

struct IFileParser
{
  using BuildSourceTargets = std::vector<
   std::pair<model::FilePtr, model::FilePtr>>;

  IFileParser() :
    _numOfSuccess(0),
    _numOfFail(0)
  {
  }

  /**
   * Returns true, when the type of a file fits to the current parser.
   */
  virtual bool accept(const std::string& path_) = 0;

  /**
   * Parser selected to parse the file is depend on the extension of argument
   * path_. See the file parsers for details.
   *
   * Deprecated since CodeCompass 4.0 (Dylan). Use the other parse method.
   *
   * This method is called from inside a transaction.
   */
  virtual std::future<ParseResult> parse(
    ParseProps parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::string& path_,
    const std::string& targetPath_,
    const std::vector<std::string>& opts_) = 0;
  
  /**
   * Parse a file (or files) based on a logged command line.
   *
   * The options vector is the argument vector of the original compiler so the
   * first element in opts_ is the parser command / executable.
   *
   * @param parseProps_ parse properties.
   * @param buildAction_ the build action of this command (already persisted).
   * @param srcMgr_ source manager.
   * @param opts_ argument vector (logged command).
   * @param files_ Source-target pairs (maybe not correct)
   */
  virtual std::future<ParseResult> parse(
    const ParseProps& parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::vector<std::string>& opts_,
    const BuildSourceTargets& files_) = 0;

  /**
   * Called at the end of parsing, after parse() calls.
   * 
   * Note:
   *  This method is called wihout an active database transaction.
   * 
   * @param parseProps_ parse properties.
   * @param numOfSuccess_ number of successfully parsed files (actions).
   * @param numOfFail_ number of failed files (actions).
   * @param indexerSvc_ indexer service.
   */
  virtual void postParse(
    ParseProps parseProps_,
    std::size_t& numOfSuccess_,
    std::size_t& numOfFail_,
    search::IndexerServiceIf& indexerSvc_)
  {
    numOfSuccess_ = _numOfSuccess;
    numOfFail_ = _numOfFail;
  }

  virtual std::string getDefaultTargetPath(const std::string& srcPath) = 0;

  virtual ~IFileParser(){}

protected:
  std::atomic<std::size_t> _numOfSuccess;
  std::atomic<std::size_t> _numOfFail;
};

class FileParser
{
public:
  static FileParser& instance()
  {
    static FileParser instance;
    return instance;
  }

  std::shared_ptr<IFileParser> getParser(const std::string& path_);
  std::vector<std::shared_ptr<IFileParser>> getParsers() { return _fileParsers; } // returns a copy!

  void registerParser(std::shared_ptr<IFileParser> fileParser_)
  {
    _fileParsers.push_back(fileParser_);
  }

  void deregisterParser(std::shared_ptr<IFileParser> fileParser_)
  {
    auto it = std::find(_fileParsers.begin(), _fileParsers.end(), fileParser_);
    if (it != _fileParsers.end())
    {
      _fileParsers.erase(it);
    }
  }

  /**
   * Parser selected to parse the file is depend on the extension of argument file_.
   * See the file parsers for details. (C++, Java is supported)
   * \arg parseProp Parsing properties eg.: project root id, filesystem root id etc.
   * \return false when no parser found for that file
   */
  std::future<ParseResult> parse(
    ParseProps parseProps_,
    model::BuildActionPtr buildAction_,
    SourceManager& srcMgr_,
    const std::string& path_,
    const std::string& targetpath_,
    const std::vector<std::string>& opts_);

  bool hasParser(const std::string& path_);

private:
  std::vector<std::shared_ptr<IFileParser>> _fileParsers;

};

} // parser
} // cc

#endif //PARSER_FILEPARSER_H
