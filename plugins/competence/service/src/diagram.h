#ifndef CC_SERVICE_COMPETENCE_DIAGRAM_H
#define CC_SERVICE_COMPETENCE_DIAGRAM_H

#include <service/competenceservice.h>
#include <projectservice/projectservice.h>

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/useremail.h>
#include <model/useremail-odb.hxx>

#include <util/graph.h>

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
    const core::FileId& fileId_,
    std::string user_,
    const std::int32_t diagramType_);

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  typedef odb::query<model::FileComprehension> FileComprehensionQuery;
  typedef odb::query<model::File> FileQuery;
  typedef odb::query<model::UserEmail> UserEmailQuery;
  typedef odb::result<model::UserEmail> UserEmailResult;

  enum DiagramType
  {
    USER,
    TEAM
  };

  void userView(
    util::Graph& graph_,
    const core::FileId& fileId_,
    std::string user_);

  std::map<util::Graph::Node, uint16_t> getFileCompetenceRates(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const std::vector<std::string>& emails_);

  void teamView(
    util::Graph& graph_,
    const core::FileId& fileId_);

  std::map<util::Graph::Node, std::string> getFileExpertNodes(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileId& fileId_,
    DiagramType type_ = TEAM);

  std::string getExpertOnFile(
    const core::FileId& fileId_);

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

  std::string generateColor(const std::string& email_);
  void setCharCodesMap();

  static const Decoration centerNodeDecoration;
  static const Decoration directoryNodeDecoration;

  static const Decoration containsEdgeDecoration;
  static const Decoration subdirEdgeDecoration;

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  CompetenceServiceHandler _compHandler;
  core::ProjectServiceHandler _projectHandler;

  const int nodeCountBorder = 15;

  static std::map<char, std::uint32_t> _charCodes;
  static std::map<std::string, std::string> _colorCodes;

};
}
}
}





#endif //CC_SERVICE_COMPETENCE_DIAGRAM_H
