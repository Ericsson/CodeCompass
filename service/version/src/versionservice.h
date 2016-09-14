#ifndef VERSIONSERVICE_H
#define VERSIONSERVICE_H

#include <mutex>
#include <cstdio>
#include <memory>
#include <functional>
#include <odb/database.hxx>

#include <boost/program_options/variables_map.hpp>

#include "version-api/VersionService.h"

namespace cc
{
namespace parser
{
//will be passed by reference to internal function
class GitRepository;
}

namespace service
{
namespace version
{
  
struct CommitListCache;

class VersionServiceHandler : virtual public VersionServiceIf {
  
public:
  VersionServiceHandler(
    std::shared_ptr<odb::database> db_,
    const boost::program_options::variables_map& config_
      = boost::program_options::variables_map());
  ~VersionServiceHandler();

  virtual void getRepositoryList(std::vector<Repository> & _return) override;
  virtual void getBlob(VersionBlob& _return, const std::string& repoId_, const std::string& hexOid_) override;
  virtual void getTree(std::vector<VersionTreeEntry> & _return, const std::string& repoId_, const std::string& hexOid_) override;
  virtual void getCommit(VersionCommit& _return, const std::string& repoId_, const std::string& hexOid_) override;
  virtual void getTag(VersionTag& _return, const std::string& repoId_, const std::string& hexOid_) override;
  virtual void getCommitListFiltered(CommitListFilteredResult& _return, const std::string& repoId_, const std::string& hexOid_, const int32_t count_, const int32_t offset_, const std::string& filter_) override;
  virtual void getReferenceList(std::vector<std::string> & _return, const std::string& repoId_) override;
  virtual void getBrancheList(std::vector<std::string> & _return, const std::string& repoId_) override;
  virtual void getTagList(std::vector<std::string> & _return, const std::string& repoId_) override;
  virtual void getActiveReference(std::string& _return, const std::string& repoId_) override;
  virtual void getReferenceTopObject(ReferenceTopObjectResult& _return, const std::string& repoId_, const std::string& branchName_) override;
  virtual void getCommitDiffAsString(std::string& _return, const std::string& repoId_, const std::string& hexOid_, const VersionDiffOptions& options_) override;
  virtual void getCommitDiffAsStringCompact(std::string& _return, const std::string& repoId_, const std::string& hexOid_, const VersionDiffOptions& options_) override;
  virtual void getCommitDiffDeltas(std::vector<VersionDiffDelta> & _return, const std::string& repoId_, const std::string& hexOid_, const VersionDiffOptions& options_) override;
  virtual void getBlobOidByPath(std::string& _return, const std::string& repoId_, const std::string& commidOid_, const std::string& path_) override;
  virtual void getBlameInfo(std::vector<VersionBlameHunk> & _return, const std::string& repoId_, const std::string& commidOid_, const std::string& path_, const std::string& localModificationsFileId_) override;
  virtual void getRepositoryByProjectPath(RepositoryByProjectPathResult& _return, const std::string& path_) override;
  virtual void getFileRevisionLog(VersionLogResult & _return, const std::string& repoId_, const std::string& branchName_, const std::string& path, const std::string& continueAtCommit_, int32_t pageSize_, const std::vector<std::string>& continueAtDrawState_) override;
  

private:

  std::string internal_getRepoPath(const std::string& repoId_);

  std::shared_ptr<odb::database> _db;
  std::string _projectDataDir;
  std::vector<Repository> _repoListCache;

  //implementaion note:
  //revWalkCache contains pointers so one should not worry about reading
  //a single pointed item while adding elements to the map itself
  //TODO destructor may segfault (?)
  std::map<std::string, std::unique_ptr<CommitListCache> > revWalkCache;
  //Mutex of revWalkCache.
  //All operations on revWalkCache itself should aquire a lock first.
  std::mutex revWalkCacheMutex;

  const CommitListCache& internal_getRevWalk(
    const std::string& repoId_,
    cc::parser::GitRepository& repo,
    const std::string& hexOid_);
  
};

} // version
} // service
} // cc


#endif