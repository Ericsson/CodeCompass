#include <service/competenceservice.h>
#include "../include/service/competenceservice.h"
#include "../../model/include/model/filecomprehension.h"
#include "../../../../model/include/model/file.h"

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
  std::shared_ptr<std::string> /*datadir_*/,
  const cc::webserver::ServerContext& context_)
  : _db(db_), _transaction(db_)
{
}

void CompetenceServiceHandler::setCompetenceRatio(std::string& return_,
  const core::FileId& fileId_,
  const int ratio_)
{
  _transaction([&, this](){
    model::FileComprehension fileComprehension;
    fileComprehension.ratio = ratio_;
    fileComprehension.file = std::make_shared<model::File>();
    fileComprehension.file->id = std::stoull(fileId_);
    _db->persist(fileComprehension);

  });
}

void CompetenceServiceHandler::loadRepositoryData(std::string& return_,
  const std::string& repoId_)
{
  _transaction([&, this](){
    RepositoryPtr repo = createRepository(repoId_);

    if (!repo)
      return;


  });
}

RepositoryPtr CompetenceServiceHandler::createRepository(const std::string& repoId_)
{
  std::string repoPath = *_datadir + "/version/" + repoId_;
  git_repository* repository = nullptr;
  int error = git_repository_open(&repository, repoPath.c_str());

  if (error)
    LOG(error) <<"Opening repository " << repoPath << " failed: " << error;

  return RepositoryPtr { repository, &git_repository_free };
}

} // competence
} // service
} // cc


