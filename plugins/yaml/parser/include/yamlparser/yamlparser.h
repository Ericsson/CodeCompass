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

  struct keyData {
    ryml::csubstr key;
    ryml::csubstr parent;
    ryml::csubstr data;
    keyData() {}
    keyData(ryml::csubstr k, ryml::csubstr p, ryml::csubstr d) : key(k), parent(p), data(d) {}

    friend std::ostream& operator<<(std::ostream &os, const keyData &kd)
    {
      os << kd.key << " " << kd.parent << " " << kd.data << std::endl;
      return os;
    }
  };

  std::vector<keyData> getDataFromFile(model::FilePtr file_) const;
  bool accept(const std::string& path_) const;

  void persistData(const std::vector<keyData>& data_, model::FileId file_);

  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
