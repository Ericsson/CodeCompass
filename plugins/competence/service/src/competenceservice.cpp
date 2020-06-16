#include <service/competenceservice.h>

#include "diagram.h"

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/useremail.h>
#include <model/useremail-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

namespace cc
{
namespace service
{
namespace competence
{

typedef odb::query<model::FileComprehension> FileComprehensionQuery;
typedef odb::query<model::UserEmail> UserEmailQuery;

CompetenceServiceHandler::CompetenceServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _datadir(std::move(datadir_)),
    _context(context_),
    _sessionManagerAccess(context_.sessionManager)
{
}

void CompetenceServiceHandler::setCompetenceRatio(
  std::string& return_,
  const core::FileId& fileId_,
  const int ratio_)
{
  _transaction([&, this]()
  {
    std::string user = getCurrentUser();

    if (user == "Anonymous")
      return;

    auto emails = _db->query<model::UserEmail>(
      UserEmailQuery::username == user);

    if (emails.empty())
      return;

    std::vector<model::UserEmail> emailList;
    for (const model::UserEmail& e : emails)
      emailList.push_back(e);

    auto file = _db->query<model::FileComprehension>(
      FileComprehensionQuery::file == std::stoull(fileId_));

    model::FileComprehension comp;
    model::UserEmail compEmail = *emailList.begin();
    bool found = false;
    for (const model::FileComprehension& f : file)
    {
      for (const model::UserEmail& e : emailList)
        if (f.userEmail == e.email && !found)
        {
          comp = f;
          compEmail = e;
          found = true;
        }

      if (found)
        break;
    }

    if (!found)
    {
      model::FileComprehension fileComprehension;
      fileComprehension.userRatio = ratio_;
      fileComprehension.repoRatio.reset();
      fileComprehension.file = std::stoull(fileId_);
      fileComprehension.inputType = model::FileComprehension::InputType::USER;
      fileComprehension.userEmail = compEmail.email;
      _db->persist(fileComprehension);
    }
    else
    {
      comp.userRatio = ratio_;
      comp.inputType = model::FileComprehension::InputType::USER;
      _db->update(comp);
    }
  });
}

void CompetenceServiceHandler::getDiagram(
  std::string& return_,
  const core::FileId& fileId_,
  const std::int32_t diagramId_)
{
  CompetenceDiagram diagram(_db, _datadir, _context);
  util::Graph graph;

  diagram.getCompetenceDiagram(graph, fileId_, getCurrentUser(), diagramId_);

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);
}

void CompetenceServiceHandler::getDiagramLegend(
  std::string& return_,
  const std::int32_t diagramId_)
{
  CompetenceDiagram diagram(_db, _datadir, _context);

  switch(diagramId_)
  {
    case 0:
      return_ = diagram.getUserViewDiagramLegend();
      break;
    case 1:
      return_ = diagram.getTeamViewDiagramLegend();
      break;
  }
}

void CompetenceServiceHandler::getUserEmailPairs(
  std::vector<UserEmail>& return_)
{
  _transaction([&, this]()
  {
    auto emails = _db->query<model::UserEmail>();
    for (const auto& e : emails)
    {
      UserEmail tmp;
      tmp.email = e.email;
      tmp.username = e.username;
      tmp.company = e.company;
      return_.push_back(tmp);
    }
  });
}

void CompetenceServiceHandler::setUserData(
  std::string& return_,
  const std::string& email_,
  const std::string& username_,
  const std::string& company_)
{
  _transaction([&, this]()
  {
    auto record = _db->query_one<model::UserEmail>(
      UserEmailQuery::email == email_);
    record.get()->username = username_;
    record.get()->company = company_;
    _db->update(record);
  });
}

std::string CompetenceServiceHandler::getCurrentUser()
{
  std::string ret;
  _sessionManagerAccess.accessSession(
    [&](webserver::Session* sess_)
    {
      ret = sess_ ? sess_->username : std::string();
    });

  return ret;
}

} // competence
} // service
} // cc


