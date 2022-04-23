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
  enum Type
  {
    KUBERNETES_CONFIG,
    DOCKERFILE,
    HELM_CHART,
    CI,
    OTHER
  };
  struct keyData {
    ryml::csubstr key;
    // ryml::csubstr parent;
    ryml::csubstr data;
    keyData() {}
    keyData(ryml::csubstr k, ryml::csubstr d) : key(k), data(d) {}

    friend std::ostream& operator<<(std::ostream &os, const keyData &kd)
    {
      os << kd.key << " " << kd.data << std::endl;
      return os;
    }
  };

  std::vector<keyData> getDataFromFile(model::FilePtr file_, Type &type) const;
  bool accept(const std::string& path_) const;
  bool isCI (std::string const &filename, std::string const &ending) 
  {
    if (filename.length() >= ending.length())
    {
      return (0 == filename.compare (filename.length() - ending.length(), ending.length(), ending));
    } 
    else
    {
      return false;
    } 
}
  // std::vector<keyData> duplicate(std::vector<keyData> kdata);

  // void persistData(const std::vector<keyData>& data_, model::FileId file_, Type type);
  void persistData(model::FilePtr file_, model::FileId fileId_);
  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
