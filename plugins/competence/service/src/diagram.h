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

  std::string getUserViewDiagramLegend();
  std::string getTeamViewDiagramLegend();
  std::string getCompanyViewLegend();
  std::string getRiskViewLegend();

private:
  typedef std::vector<std::pair<std::string, std::string>> Decoration;

  typedef odb::query<model::FileComprehension> FileComprehensionQuery;
  typedef odb::result<model::FileComprehension> FileComprehensionResult;
  typedef odb::query<model::File> FileQuery;
  typedef odb::query<model::UserEmail> UserEmailQuery;

  /**
   * This method generates a graph for a single file or
   * a directory recursively to show the competence rates
   * of the logged in user.
   */
  void userViewDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_,
    std::string &user_);

  std::map<util::Graph::Node, int16_t> getFileCompetenceRates(
    util::Graph& graph_,
    const util::Graph::Node& node_,
    const std::vector<std::string>& emails_);

  /**
   * This method generates a graph for a single file or
   * a directory recursively to show who the most knowledgeable
   * developer is on each file.
   */
  void teamViewDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_);

  std::map<util::Graph::Node, std::string> getFileExpertNodes(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This method generates a graph for a single file or
   * a directory recursively to show the expert company
   * based on the affiliation of the most knowledgeable
   * developer regarding the file.
   */
  void personalCompanyViewDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_);

  std::map<util::Graph::Node, std::string> getCompanyExpertNodes(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This method generates a graph for a single file or
   * a directory recursively to show the expert company
   * based on the average competence of the developers of
   * the company who made relevant changes to the file.
   * -- WIP, algorithm is not final --
   */
  void accumulatedCompanyViewDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_);

  std::map<util::Graph::Node, std::string> getAccumulatedCompanyExpertNodes(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  /**
   * This method generates a graph for a single file or
   * a directory recursively to show which files are at
   * risk of forgetting. At least 2 devs with >=50%
   * understanding mean no risk, 1 dev means low risk,
   * any lower means high risk.
   */
  void riskViewDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_);

  std::map<util::Graph::Node, short> getFileRiskNodes(
    util::Graph& graph_,
    const util::Graph::Node& node_);

  void primeFilesDiagram(
    util::Graph &graph_,
    const core::FileId &fileId_);

  /**
   * Returns the expert company for a file.
   */
  std::string maxCompanyCompetence(FileComprehensionResult& result);

  /**
   * Returns the expert user and their competence data for a file.
   */
  model::FileComprehension maxCompetence(FileComprehensionResult& result);

  /**
   * Returns the affiliation of a user.
   */
  std::string getCompany(const std::string& email);

  /**
   * Converts a percentage value to a hex color code.
   */
  std::string rateToColor(int16_t rate);

  /**
   * Converts a risk value to red, yellow or green
   * hex color code.
   */
  std::string riskCounterToColor(short rate);

  /**
   * Helper methods for graph building.
   */
  util::Graph::Node addNode(
    util::Graph& graph_,
    const core::FileInfo& fileInfo_);

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
  void setCharCodes();
  void setUserColorCodes();
  void setCompanyColorCodes();

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  CompetenceServiceHandler _compHandler;
  core::ProjectServiceHandler _projectHandler;

  const int _nodeCountLimit = 15;
  const std::string _white = "#ffffff";
  const std::string _grey = "#D3D3D3";

  static std::map<char, std::uint32_t> _charCodes;
  static std::map<std::string, std::string> _userColorCodes;
  static std::map<std::string, std::string> _companyColorCodes;
  static std::set<std::string> _currentTeamViewUsers;
  static std::set<std::string> _currentCompanies;

  static const Decoration directoryNodeDecoration;

  static const Decoration containsEdgeDecoration;
  static const Decoration subdirEdgeDecoration;

};
}
}
}





#endif //CC_SERVICE_COMPETENCE_DIAGRAM_H
