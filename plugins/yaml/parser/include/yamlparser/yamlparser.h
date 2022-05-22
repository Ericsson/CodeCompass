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
    std::string key;
    std::string parent;
    std::string data;
    keyData() {}
    keyData(ryml::csubstr k, ryml::csubstr p, ryml::csubstr d) : key(k.str, k.len), parent(p.str, p.len), data(d.str, d.len) {}

    friend std::ostream& operator<<(std::ostream &os, const keyData &kd)
    {
      os << kd.key << " " << kd.parent << "  " << kd.data << std::endl;
      return os;
    }
  };
  void getstr(ryml::NodeRef node, ryml::csubstr parent, std::vector<keyData> &vec);
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

  void persistData(model::FilePtr file_);
  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
