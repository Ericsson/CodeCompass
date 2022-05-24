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
  struct keyData
  {
    std::string key;
    std::string parent;
    std::string data;
    keyData() {}
    keyData(ryml::csubstr k, ryml::csubstr p, ryml::csubstr d)
     : key(k.str, k.len), parent(p.str, p.len), data(d.str, d.len) {}
  };

  util::DirIterCallback getParserCallback();

  std::string getDataFromNode(const std::string &node_, const bool isSeq_);

  void getKeyDataFromTree(
    ryml::NodeRef node_,
    ryml::csubstr parent_,
    std::vector<keyData>& dataVec_);
  
  bool accept(const std::string& path_) const;

  bool isCIFile (std::string const& filename_, std::string const& ending_)
  {
    if (filename_.length() >= ending_.length())
    {
      return (0 == filename_.compare (
        filename_.length() - ending_.length(), ending_.length(), ending_));
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
