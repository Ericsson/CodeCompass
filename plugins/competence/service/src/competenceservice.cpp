#include <service/competenceservice.h>

#include "diagram.h"

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

namespace cc
{
namespace service
{
namespace competence
{

CompetenceServiceHandler::CompetenceServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
  : _db(db_),
    _transaction(db_),
    _datadir(datadir_),
    _context(context_),
    _sessionManagerAccess(context_.sessionManager)
{
}

void CompetenceServiceHandler::setCompetenceRatio(std::string& return_,
  const core::FileId& fileId_,
  const int ratio_)
{
  _transaction([&, this]()
  {
    auto file = _db->query<model::FileComprehension>(
      odb::query<model::FileComprehension>::file == std::stoull(fileId_));

    if (!file.empty())
    {
      for (model::FileComprehension& comp : file)
      {
        comp.userRatio = ratio_;
        comp.inputType = model::FileComprehension::InputType::USER;
        _db->update(comp);
      }
    }
    else
    {
      model::FileComprehension fileComprehension;
      fileComprehension.userRatio = ratio_;
      fileComprehension.repoRatio.reset();
      fileComprehension.file = std::stoull(fileId_);
      fileComprehension.inputType = model::FileComprehension::InputType::USER;
      _db->persist(fileComprehension);
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
/*
void CompetenceServiceHandler::getDiagramTypes(
  std::map<std::string, std::int32_t>& return_,
  const core::FileId& fileId_)
{
  model::FilePtr file = _transaction([&, this](){
    return _db->query_one<model::File>(
      odb::query<cc::model::File>::id == std::stoull(fileId_));
  });

  if (file->type == model::File::DIRECTORY_TYPE)
  {
    return_["Directory competence of user"] = DIRECTORY_COMPETENCE;
  }
  else
  {
    return_["File competence of user"] = FILE_COMPETENCE;
  }
}*/

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


