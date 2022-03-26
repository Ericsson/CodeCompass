#ifndef CC_PARSER_YAML_PARSER_H
#define CC_PARSER_YAML_PARSER_H

#include <atomic>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <util/parserutil.h>

namespace cc
{
namespace parser
{

class YamlParser : public AbstractParser
{
public:
  YamlParser(ParserContext& ctx_);
  virtual bool cleanupDatabase() override;
  virtual bool parse() override;

private:
  util::DirIterCallback getParserCallback();

  struct Loc
  {
    Loc() : originalLines(0), nonblankLines(0), codeLines(0) {}

    unsigned originalLines;
    unsigned nonblankLines;
    unsigned codeLines;
  };

  Loc getLocFromFile(model::FilePtr file_) const;

  void persistLoc(const Loc& loc_, model::FileId file_);

  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
