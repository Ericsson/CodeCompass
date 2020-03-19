#include <service/competenceservice.h>
#include "../../model/include/model/filecomprehension.h"
#include "../../../../model/include/model/file.h"
#include "../include/service/competenceservice.h"
#include "diagram.h"

#include <model/filecomprehension.h>
#include <model/filecomprehension-odb.hxx>
#include <model/file.h>

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
    _context(context_)
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

  switch (diagramId_)
  {
    case FILE:
      diagram.getFileCompetenceDiagram(graph, fileId_);
      break;

    case DIRECTORY:
      diagram.getDirectoryCompetenceDiagram(graph, fileId_);
      break;
  }

  if (graph.nodeCount() != 0)
    return_ = graph.output(util::Graph::SVG);
}

} // competence
} // service
} // cc


