#ifndef CC_SERVICE_COMPETENCE_DIAGRAM_H
#define CC_SERVICE_COMPETENCE_DIAGRAM_H

#include <service/competenceservice.h>
#include <projectservice/projectservice.h>
#include <util/graph.h>

#include "../../../../../../../cc/build/service/language/gen-cpp/language_types.h"

namespace cc
{
namespace service
{
namespace competence
{

class CompetenceDiagram
{
public:
  CompetenceDiagram(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getCompetenceDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

  void getTeamViewDiagram(
    util::Graph& graph_,
    const core::FileId& fileId_);

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  std::map<util::Graph::Node, uint16_t> getFileCompetenceRates(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

  std::string rateToColor(uint16_t rate);

  std::vector<util::Graph::Node> getSubDirs(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  void decorateNode(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const Decoration& decoration_) const;

  void decorateEdge(
    util::Graph& graph_,
    const util::Graph::Edge& edge_,
    const Decoration& decoration_) const;

  static const Decoration centerNodeDecoration;
  static const Decoration directoryNodeDecoration;

  static const Decoration containsEdgeDecoration;
  static const Decoration subdirEdgeDecoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  CompetenceServiceHandler _compHandler;
  core::ProjectServiceHandler _projectHandler;

};
}
}
}





#endif //CC_SERVICE_COMPETENCE_DIAGRAM_H
