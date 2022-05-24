#ifndef CC_SERVICE_YAML_H
#define CC_SERVICE_YAML_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/yaml.h>
#include <model/yaml-odb.hxx>

#include <model/yamlcontent.h>
#include <model/yamlcontent-odb.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <projectservice/projectservice.h>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>
#include <util/graph.h>
#include <util/legendbuilder.h>
#include <util/util.h>

#include <YamlService.h>

namespace cc
{
namespace service
{
namespace yaml
{

class YamlServiceHandler  : virtual public YamlServiceIf
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
