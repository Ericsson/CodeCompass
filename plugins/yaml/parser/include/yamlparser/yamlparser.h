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
  ~YamlParser();
  virtual bool cleanupDatabase() override;
  virtual bool parse() override;

private:
  /*struct keyData
  {
    std::string key;
    std::string parent;
    std::string data;
    keyData() {}
    keyData(ryml::csubstr k, ryml::csubstr p, ryml::csubstr d)
     : key(k.str, k.len), parent(p.str, p.len), data(d.str, d.len) {}
  };*/

  void processFileType(model::FilePtr& file_, YAML::Node& loadedFile);
  void processRootKeys(model::FilePtr& file_, YAML::Node& loadedFile);

  bool collectAstNodes(model::FilePtr file_);

  void chooseCoreNodeType(
    YAML::Node& node_,
    model::FilePtr file_,
    model::YamlAstNode::SymbolType symbolType_);

  void processAtomicNode(
    YAML::Node& node_,
    model::FilePtr file_,
    model::YamlAstNode::SymbolType symbolType_,
    model::YamlAstNode::AstType astType_);
  void processMap(
    YAML::Node& node_,
    model::FilePtr file_,
    model::YamlAstNode::SymbolType symbolType_);
  void processSequence(
    YAML::Node& node_,
    model::FilePtr file_,
    model::YamlAstNode::SymbolType symbolType_);

  model::Range getNodeLocation(YAML::Node& node_);

  util::DirIterCallback getParserCallback();

  std::string getDataFromNode(const std::string &node_, const bool isSeq_);

  /*void getKeyDataFromTree(
    YAML::Node node_,
    ryml::csubstr parent_,
    std::vector<keyData>& dataVec_);*/
  
  bool accept(const std::string& path_) const;

  bool isCIFile(std::string const& filename_, std::string const& ending_);

  //void persistData(model::FilePtr file_);
  //model::FileLoc nodeLocation(ryml::Parser&, ryml::NodeRef&);

  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
  std::vector<model::YamlAstNodePtr> _astNodes;
  std::vector<model::YamlFilePtr> _yamlFiles;
  std::vector<model::YamlContentPtr> _rootPairs;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
