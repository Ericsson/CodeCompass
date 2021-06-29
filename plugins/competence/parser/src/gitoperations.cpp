#include "competenceparser/gitoperations.h"

#include <util/logutil.h>

namespace cc
{
namespace parser
{

GitOperations::GitOperations()
{
  git_libgit2_init();
}

GitOperations::~GitOperations()
{
  git_libgit2_shutdown();
}

RevWalkPtr GitOperations::createRevWalk(git_repository* repo_)
{
  git_revwalk* walker = nullptr;
  int error = git_revwalk_new(&walker, repo_);

  if (error)
    LOG(error) << "Creating revision walker failed: " << error;

  return RevWalkPtr { walker, &git_revwalk_free };
}

RepositoryPtr GitOperations::createRepository(
  const boost::filesystem::path& repoPath_)
{
  git_repository* repository = nullptr;
  int error = git_repository_open(&repository, repoPath_.c_str());

  if (error)
    LOG(error) << "Opening repository " << repoPath_ << " failed: " << error;

  return RepositoryPtr { repository, &git_repository_free };
}

BlamePtr GitOperations::createBlame(
  git_repository* repo_,
  const std::string& path_,
  git_blame_options* opts_)
{
  git_blame* blame = nullptr;
  int error = git_blame_file(&blame, repo_, path_.c_str(), opts_);

  if (error)
    LOG(error) << "Getting blame object failed: " << error;

  return BlamePtr { blame, &git_blame_free };
}

CommitPtr GitOperations::createCommit(
  git_repository* repo_,
  const git_oid& id_)
{
  git_commit* commit = nullptr;
  int error = git_commit_lookup(&commit, repo_, &id_);

  if (error)
    LOG(error) << "Getting commit failed: " << error;

  return CommitPtr { commit, &git_commit_free };
}

CommitPtr GitOperations::createParentCommit(
  git_commit* commit_)
{
  git_commit* parent = nullptr;
  int error = git_commit_parent(&parent, commit_, 0);

  if (error)
    LOG(error) << "Getting commit parent failed: " << error;

  return CommitPtr { parent, &git_commit_free };
}

TreePtr GitOperations::createTree(
  git_commit* commit_)
{
  git_tree* tree = nullptr;
  int error = git_commit_tree(&tree, commit_);

  if (error)
    LOG(error) << "Getting commit tree failed: " << error;

  return TreePtr { tree, &git_tree_free };
}

DiffPtr GitOperations::createDiffTree(
  git_repository* repo_,
  git_tree* first_,
  git_tree* second_)
{
  git_diff* diff = nullptr;
  int error = git_diff_tree_to_tree(&diff, repo_, first_, second_, nullptr);

  if (error)
    LOG(error) << "Getting commit diff failed: " << error;

  return DiffPtr { diff, &git_diff_free };
}

BlameOptsPtr GitOperations::createBlameOpts(const git_oid& newCommitOid_)
{
  git_blame_options* blameOpts = new git_blame_options;
  git_blame_init_options(blameOpts, GIT_BLAME_OPTIONS_VERSION);
  blameOpts->newest_commit = newCommitOid_;
  return BlameOptsPtr { blameOpts };
}

} // parser
} // cc