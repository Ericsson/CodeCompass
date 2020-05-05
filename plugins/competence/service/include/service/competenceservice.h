#ifndef PROJECT_COMPETENCESERVICE_H
#define PROJECT_COMPETENCESERVICE_H

#include <CompetenceService.h>

#include <memory>

#include <git2.h>

#include <model/file.h>
#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

namespace cc
{
namespace service
{
namespace competence
{

class CompetenceServiceHandler : virtual public CompetenceServiceIf
{
  friend class CompetenceDiagram;

public:
  CompetenceServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void setCompetenceRatio(
    std::string& return_,
    const core::FileId& fileId_,
    const int ratio_) override;

  void getDiagram(
    std::string& return_,
    const core::FileId& fileId_,
    const std::int32_t diagramId_) override;

  void getDiagramTypes(
    std::map<std::string, std::int32_t>& return_,
    const core::FileId& fileId_);

private:
  enum DiagramType
  {
    FILE_COMPETENCE,
    DIRECTORY_COMPETENCE
  };

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  std::shared_ptr<std::string> _datadir;
  const cc::webserver::ServerContext& _context;
};

} // competence
} // service
} // cc


#endif //PROJECT_COMPETENCESERVICE_H
