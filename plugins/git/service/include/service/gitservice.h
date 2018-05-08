#ifndef CC_SERVICE_GITSERVICE_H
#define CC_SERVICE_GITSERVICE_H

#include <string>
#include <vector>
#include <memory>

#include <git2.h>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <projectservice/projectservice.h>

#include <GitService.h>

namespace cc
{

namespace service
{

namespace git
{

typedef std::unique_ptr<git_repository, decltype(&git_repository_free)> RepositoryPtr;
typedef std::unique_ptr<git_revwalk, decltype(&git_revwalk_free)> RevWalkPtr;
typedef std::unique_ptr<git_commit, decltype(&git_commit_free)> CommitPtr;
typedef std::unique_ptr<git_tree, decltype(&git_tree_free)> TreePtr;
typedef std::unique_ptr<git_tree_entry, decltype(&git_tree_entry_free)> TreeEntryPtr;
typedef std::unique_ptr<git_tag, decltype(&git_tag_free)> TagPtr;
typedef std::unique_ptr<git_object, decltype(&git_object_free)> ObjectPtr;
typedef std::unique_ptr<git_diff, decltype(&git_diff_free)> DiffPtr;
typedef std::unique_ptr<git_reference, decltype(&git_reference_free)> ReferencePtr;
typedef std::unique_ptr<git_blob, decltype(&git_blob_free)> BlobPtr;
typedef std::unique_ptr<git_blame, decltype(&git_blame_free)> BlamePtr;
typedef std::unique_ptr<git_blame_options> BlameOptsPtr;

class GitServiceHandler: virtual public GitServiceIf
{
public:
  GitServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  ~GitServiceHandler();

  virtual void getRepositoryList(
    std::vector<GitRepository>& return_) override;

  virtual void getRepositoryByProjectPath(
    RepositoryByProjectPathResult& return_,
    const std::string& path_) override;

  virtual void getBlobOidByPath(
    std::string& return_,
    const std::string& repoId_,
    const std::string& hexOid_,
    const std::string& path_) override;

  virtual void getBlobContent(
    std::string& return_,
    const std::string& repoId_,
    const std::string& hexOid_) override;

  virtual void getBlameInfo(
    std::vector<GitBlameHunk> & _return,
    const std::string& repoId_,
    const std::string& hexOid_,
    const std::string& path_,
    const std::string& localModificationsFileId_) override;

  virtual void getCommit(
    GitCommit& return_,
    const std::string& repoId_,
    const std::string& hexOid_) override;

  virtual void getTag(
    GitTag& return_,
    const std::string& repoId_,
    const std::string& hexOid_) override;

  virtual void getCommitListFiltered(
    CommitListFilteredResult& return_,
    const std::string& repoId_,
    const std::string& hexOid_,
    const int32_t count_,
    const int32_t offset_,
    const std::string& filter_) override;

  virtual void getReferenceList(
    std::vector<std::string>& return_,
    const std::string& repoId_) override;

  virtual void getBranchList(
    std::vector<std::string>& return_,
    const std::string& repoId_) override;

  virtual void getTagList(
    std::vector<std::string>& return_,
    const std::string& repoId_) override;

  virtual void getReferenceTopObject(
    ReferenceTopObjectResult& return_,
    const std::string& repoId_,
    const std::string& branchName_) override;

  virtual void getCommitDiffAsString(
    std::string& return_,
    const std::string& repoId_,
    const std::string& hexOid_,
    const GitDiffOptions& options_,
    const bool isCompact_ = false) override;

private:
  /**
   * Returns the absolute path of the repository identified by repository id.
   */
  std::string getRepoPath(const std::string& repoId_) const;

  /**
   * Format a string into a git_oid.
   */
  git_oid gitOidFromStr(const std::string& hexOid_);

  /**
   * Format a git_oid into a string.
   */
  std::string gitOidToString(const git_oid* oid_);

  /**
   * This function creates a signature (e.g. for committers, taggers, etc).
   */
  std::string gitSignatureToString(const git_signature* sig_);

  /**
   * Open a git repository. The 'repoId_' argument must be a valid
   * repository id.
   */
  RepositoryPtr createRepository(const std::string& repoId_);

  /**
   * Retrieve and resolve the reference pointed at by HEAD.
   */
  ReferencePtr createRepositoryHead(git_repository* repo_);

  /**
   * Allocate a new revision walker to iterate through a repo.
   */
  RevWalkPtr createRevWalk(git_repository* repo_);

  /**
   * Lookup a commit object from a repository.
   */
  CommitPtr createCommit(git_repository* repo_, const git_oid& id_);

  /**
   * Get the tree pointed to by a commit.
   */
  TreePtr createTree(git_commit* commit_);

  /**
   * Lookup a tree object from the repository.
   */
  TreePtr createTree(git_repository* repo_, const git_oid& id_);

  /**
   * Retrieve a tree entry contained in a tree or in any of its subtrees,
   * given its relative path.
   */
  TreeEntryPtr createTreeEntry(git_tree* tree_, const std::string& path_);

  /**
   * Lookup a tag object from the repository.
   */
  TagPtr createTag(git_repository* repo_, const git_oid& oid_);

  /**
   * Lookup a reference to one of the objects in a repository.
   */
  ObjectPtr createObject(git_repository* repo_, const git_oid& oid_);

  /**
   * Lookup a blob object from a repository.
   */
  BlobPtr createBlob(git_repository* repo_, git_oid* oid_);

  /**
   * Get the blame for a single file.
   */
  BlamePtr createBlame(
    git_repository* repo_,
    const std::string& path_,
    git_blame_options* opts_);

  /**
   * Get blame data for a file that has been modified in memory.
   */
  BlamePtr getBlameData(const BlamePtr& blame_, const std::string content_);

  /**
   * Create a git blame option.
   */
  BlameOptsPtr createBlameOpts(const git_oid& newCommitOid_);

  /**
   * Create a diff with the difference between two tree objects.
   */
  DiffPtr createDiff(
    git_repository *repo,
    git_tree *oldTree,
    git_tree *newTree,
    git_diff_options *opts);

  /**
   * Set thrift GitCommit object by commit.
   */
  void setCommitData(
    GitCommit& return_,
    const std::string& repoId_,
    git_commit* commit_);

  /**
   * Get the specified parent of the commit.
   */
  std::vector<std::string> getParents(git_commit* commit_);

  /**
   * Iterate over a diff generating formatted text output.
   */
  std::string gitDiffToString(git_diff* diff_, bool isCompact_ = false);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::shared_ptr<std::string> _datadir;

  core::ProjectServiceHandler _projectHandler;
};

} //namespace git
} //namespace service
} //namespace cc

#endif // CC_SERVICE_GITSERVICE_H
