#ifndef PROJECT_COMPETENCESERVICE_H
#define PROJECT_COMPETENCESERVICE_H

#include <memory>

#include <git2.h>

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

typedef std::unique_ptr<git_blame, decltype(&git_blame_free)> BlamePtr;
typedef std::unique_ptr<git_blame_options> BlameOptsPtr;
typedef std::unique_ptr<git_commit, decltype(&git_commit_free)> CommitPtr;
typedef std::unique_ptr<git_repository, decltype(&git_repository_free)> RepositoryPtr;

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

  void loadRepositoryData(std::string& return_,
    const core::FileId& fileId_,
    const std::string& repoId_,
    const std::string& hexOid_,
    const std::string& path_,
    const std::string& user_ = "none");

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::shared_ptr<std::string> _datadir;

  BlamePtr createBlame(
    git_repository* repo_,
    const std::string& path_,
    git_blame_options* opts_);
  BlameOptsPtr createBlameOpts(const git_oid& newCommitOid_);
  RepositoryPtr createRepository(const std::string& repoId_);
  CommitPtr createCommit(git_repository *repo_,
    const git_oid& id_);
  git_oid gitOidFromStr(const std::string& hexOid_);
  std::string gitOidToString(const git_oid* oid_);

};

} // competence
} // service
} // cc


#endif //PROJECT_COMPETENCESERVICE_H
