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
  struct ParseJob
  {
    std::string path;

    std::size_t index;

    ParseJob(const std::string& path_, std::size_t index_)
      : path(path_), index(index_) {}

    ParseJob(const ParseJob&) = default;
  };

  struct CleanupJob
  {
    std::string path;

    std::size_t index;

    CleanupJob(const std::string& path, std::size_t index)
      : path(path), index(index) {}
  };

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

  //void persistData(model::FilePtr file_);
  //model::FileLoc nodeLocation(ryml::Parser&, ryml::NodeRef&);

  std::unordered_set<model::FileId> _fileIdCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
  std::vector<model::YamlAstNodePtr> _astNodes;
  std::vector<model::YamlFilePtr> _yamlFiles;
  std::vector<model::YamlContentPtr> _rootPairs;
  std::vector<model::BuildLog> _buildLogs;

  std::mutex _mutex;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
