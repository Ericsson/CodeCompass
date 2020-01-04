#ifndef PROJECT_COMPETENCESERVICE_H
#define PROJECT_COMPETENCESERVICE_H

#include <memory>

#include <model/file.h>
#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <CompetenceService.h>

namespace cc
{
namespace service
{
namespace competence
{

class CompetenceServiceHandler : virtual public CompetenceServiceIf
{
public:
  CompetenceServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void setCompetenceRatio(std::string& return_,
    const core::FileId& fileId_,
    const int ratio_) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
};

} // competence
} // service
} // cc


#endif //PROJECT_COMPETENCESERVICE_H
