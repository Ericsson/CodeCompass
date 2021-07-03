#ifndef CC_PARSER_GITOPERATIONS_H
#define CC_PARSER_GITOPERATIONS_H

#include <memory>

#include <git2.h>
#include <boost/filesystem/path.hpp>

namespace cc
{
namespace parser
{

typedef std::unique_ptr<git_blame, decltype(&git_blame_free)> BlamePtr;
typedef std::unique_ptr<git_blame_options> BlameOptsPtr;
typedef std::unique_ptr<git_blob, decltype(&git_blob_free)> BlobPtr;
typedef std::unique_ptr<git_commit, decltype(&git_commit_free)> CommitPtr;
typedef std::unique_ptr<git_diff, decltype(&git_diff_free)> DiffPtr;
typedef std::unique_ptr<git_repository, decltype(&git_repository_free)> RepositoryPtr;
typedef std::unique_ptr<git_revwalk, decltype(&git_revwalk_free)> RevWalkPtr;
typedef std::unique_ptr<git_tree, decltype(&git_tree_free)> TreePtr;
typedef std::unique_ptr<git_tree_entry, decltype(&git_tree_entry_free)> TreeEntryPtr;

struct GitSignature
{
  std::string name;
  std::string email;
  std::uint64_t time;
};

struct GitBlameHunk
{
  std::uint32_t linesInHunk;       /**< The number of lines in this hunk. */
  GitSignature finalSignature;     /**< Signature of the commit who this line
                                   last changed. */
};

class GitOperations
{
public:
  GitOperations();
  ~GitOperations();

  RevWalkPtr createRevWalk(git_repository* repo_);

  BlamePtr createBlame(
    git_repository* repo_,
    const std::string& path_,
    git_blame_options* opts_);

  BlameOptsPtr createBlameOpts(const git_oid& newCommitOid_);

  RepositoryPtr createRepository(
    const boost::filesystem::path& repoPath_);

  CommitPtr createCommit(
    git_repository *repo_,
    const git_oid& id_);

  CommitPtr createParentCommit(
    git_commit* commit_);

  TreePtr createTree(
    git_commit* commit_);

  DiffPtr createDiffTree(
    git_repository* repo_,
    git_tree* first_,
    git_tree* second_);

  unsigned int getCommitParentCount(
    git_commit* commit);

  BlobPtr createBlob(
    git_repository* repo_,
    const git_oid* oid_);

  void freeSignature(
    const git_signature* signature);

private:
  git_repository* _repo;
};
}
}

#endif // CC_PARSER_GITOPERATIONS_H
