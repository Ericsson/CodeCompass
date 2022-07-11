#ifndef CC_SERVICE_YAML_H
#define CC_SERVICE_YAML_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/yamlfile.h>
#include <model/yamlfile-odb.hxx>

#include <model/yamlcontent.h>
#include <model/yamlcontent-odb.hxx>

#include <model/yamlastnode.h>
#include <model/yamlastnode-odb.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <projectservice/projectservice.h>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>
#include <util/graph.h>
#include <util/legendbuilder.h>
#include <util/util.h>

#include <LanguageService.h>
//#include <YamlService.h>

namespace cc
{
namespace service
{
namespace language
{

class YamlServiceHandler  : virtual public LanguageServiceIf
{
public:
  YamlServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getYamlFileDiagram(
    std::string& return_,
    const core::FileId& fileId_);

  void getYamlFileInfo(
    std::string& return_,
    const core::FileId& fileId_);

  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

  std::string getLastNParts(const std::string& path_, std::size_t n_);


  void getFileTypes(std::vector<std::string>& return_) override;

  void getAstNodeInfo(
    AstNodeInfo& return_,
    const core::AstNodeId& astNodeId_) override;

  void getAstNodeInfoByPosition(
    AstNodeInfo& return_,
    const core::FilePosition& fpos_) override;

  void getSourceText(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDocumentation(
    std::string& return_,
    const core::AstNodeId& astNodeId_) override;

  void getProperties(
    std::map<std::string, std::string>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId_) override;

  void getDiagram(
    std::string& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t diagramId_) override;

  void getDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getFileDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override;

  void getFileDiagram(
    std::string& return_,
    const core::FileId& fileId_,
    const int32_t diagramId_) override;

  void getFileDiagramLegend(
    std::string& return_,
    const std::int32_t diagramId_) override;

  void getReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::AstNodeId& astNodeId) override;

  void getReferences(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::vector<std::string>& tags_) override;

  std::int32_t getReferenceCount(
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_) override;

  void getReferencesInFile(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const core::FileId& fileId_,
    const std::vector<std::string>& tags_) override;

  void getReferencesPage(
    std::vector<AstNodeInfo>& return_,
    const core::AstNodeId& astNodeId_,
    const std::int32_t referenceId_,
    const std::int32_t pageSize_,
    const std::int32_t pageNo_) override;

  void getFileReferenceTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_) override;

  void getFileReferences(
    std::vector<AstNodeInfo>& return_,
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override;

  std::int32_t getFileReferenceCount(
    const core::FileId& fileId_,
    const std::int32_t referenceId_) override;

  void getSyntaxHighlight(
    std::vector<SyntaxHighlight>& return_,
    const core::FileRange& range_) override;

  model::YamlAstNode queryYamlAstNode(
    const core::AstNodeId& astNodeId_);

  std::map<model::YamlAstNodeId, std::vector<std::string>>
  getTags(const std::vector<model::YamlAstNode>& nodes_);

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  static const Decoration sourceFileNodeDecoration;
  static const Decoration binaryFileNodeDecoration;
  static const Decoration directoryNodeDecoration;

  core::ProjectServiceHandler _projectService;

  void decorateNode(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

};

} // yaml
} // service
} // cc

#endif
