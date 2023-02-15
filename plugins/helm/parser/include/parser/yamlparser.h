#ifndef CC_PARSER_YAML_PARSER_H
#define CC_PARSER_YAML_PARSER_H

#include <atomic>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

#include <util/parserutil.h>

#include "yaml-cpp/yaml.h"

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
  /**
   * This method classifies a YAML file according to the purpose
   * it serves, e.g. Helm chart or other type of configuration file.
   * The classification is done based on naming conventions.
   */
  void processFileType(
    model::FilePtr& file_,
    YAML::Node& loadedFile);

  void processIntegrationChart(
    model::FilePtr& file_,
    YAML::Node& loadedFile);

  /**
   * The first-level keys in a YAML files usually have great significance,
   * so they should be collected in a separate collection.
   * @param file_
   * @param loadedFile
   */
  void processRootKeys(
    model::FilePtr& file_,
    YAML::Node& loadedFile);

  bool collectAstNodes(model::FilePtr file_);

  /**
   * This method is used to decide the type of root keys and values.
   */
  void chooseCoreNodeType(
    YAML::Node& node_,
    model::FilePtr file_,
    model::YamlAstNode::SymbolType symbolType_);

  /**
   * These methods handle the different key and value types
   * in YAML files.
   */
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

  /**
   * A method to recursively traverse the input directory and
   * find YAML files.
   */
  util::DirIterCallback getParserCallback();

  bool accept(const std::string& path_) const;

  std::unordered_set<model::FileId> _fileIdCache;
  std::map<std::string, YAML::Node> _fileAstCache;
  std::map<std::string, YAML::Node> _valuesAstCache;
  std::unique_ptr<util::JobQueueThreadPool<std::string>> _pool;
  std::atomic<int> _visitedFileCount;
  std::vector<model::YamlAstNodePtr> _astNodes;
  std::vector<model::YamlFilePtr> _yamlFiles;
  std::vector<model::YamlContentPtr> _rootPairs;
  std::vector<model::BuildLog> _buildLogs;
  std::vector<std::string> _processedMS;

  std::mutex _mutex;
  bool _areDependenciesListed;
};

} // namespace parser
} // namespace cc

#endif // CC_PARSER_YAML_PARSER_H
