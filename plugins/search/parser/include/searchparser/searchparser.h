#ifndef CC_PARSER_SEARCHPARSER_H
#define CC_PARSER_SEARCHPARSER_H

#include <magic.h>

#include <util/parserutil.h>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

class IndexerProcess;

class SearchParser : public AbstractParser
{
public:
  SearchParser(ParserContext& ctx_);
  virtual ~SearchParser();

  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override;

private:
  void postParse();
  util::DirIterCallback getParserCallback(const std::string& path_);
  bool shouldHandle(const std::string& path_);

private:
  /**
   * Java index process.
   */
  std::unique_ptr<IndexerProcess> _indexProcess;

  /**
   * libmagic handler for mime types.
   */
  ::magic_t _fileMagic;

  /**
   * Directory of search database.
   */
  std::string _searchDatabase;

  /**
   * Directories which have to be skipped during the parse.
   */
  std::vector<std::string> _skipDirectories;
};

} // parser
} // cc

#endif //CC_PARSER_SEARCHPARSER_H
