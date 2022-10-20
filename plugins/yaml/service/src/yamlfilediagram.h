#ifndef CC_SERVICE_LANGUAGE_YAMLFILEDIAGRAM_H
#define CC_SERVICE_LANGUAGE_YAMLFILEDIAGRAM_H

#include <model/microservice.h>

#include <service/yamlservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

namespace cc
{
namespace service
{
namespace language
{

typedef odb::result<model::File> FilePathResult;
typedef odb::query<model::File> FilePathQuery;
typedef odb::query<model::YamlFile> YamlQuery;
typedef odb::result<model::YamlFile> YamlResult;
typedef odb::result<model::YamlContent> YamlContentResult;
typedef odb::query<model::YamlContent> YamlContentQuery;

class YamlFileDiagram
{
public:
  YamlFileDiagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getYamlFileInfo(
    util::Graph& graph_,
    const core::FileId& fileId_);

  void getYamlFileDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  void getMicroserviceDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  void getDependencyDiagram(
    util::Graph& graph_,
    const language::MicroserviceId& serviceId_);

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  std::vector<util::Graph::Node> getMicroservices(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  std::vector<util::Graph::Node> getDependencies(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  std::vector<util::Graph::Node> getRevDependencies(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This method should return the relations between
   * previously detected microservices. It works if
   * the user chooses a file within a microservice.
   */
  std::vector<util::Graph::Node> getDependentServices(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    bool reverse_ = false);

  std::multimap<model::MicroserviceId, std::string> getDependentServiceIds(
    util::Graph&,
    const util::Graph::Node& node_,
    bool reverse_);

  std::string graphHtmlTag(
    const std::string& tag_,
    const std::string& content_,
    const std::string& attr_ = "");

  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

  util::Graph::Node addNode(
    util::Graph& graph_,
    const model::Microservice& service_);

  std::string getLastNParts(
    const std::string& path_,
    std::size_t n_);

  void decorateNode(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

  void decorateEdge(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

  static const Decoration sourceFileNodeDecoration;
  static const Decoration binaryFileNodeDecoration;
  static const Decoration directoryNodeDecoration;
  static const Decoration microserviceNodeDecoration;

  static const Decoration dependsEdgeDecoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  YamlServiceHandler _yamlHandler;
  core::ProjectServiceHandler _projectHandler;
};

} // language
} // service
} // cc


#endif //CC_SERVICE_LANGUAGE_YAMLFILEDIAGRAM_H
