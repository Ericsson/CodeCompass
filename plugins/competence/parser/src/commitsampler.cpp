#include "competenceparser/commitsampler.h"

#include <model/sampledata.h>
#include <model/sampledata-odb.hxx>

#include <parser/sourcemanager.h>

#include <util/logutil.h>
#include <util/odbtransaction.h>

namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

GitOperations CommitSampler::_gitOps;

CommitSampler::CommitSampler(ParserContext& ctx_)
  : _ctx(ctx_)
{

}

void CommitSampler::commitSampling(
  const std::string& root_,
  boost::filesystem::path& repoPath_)
{
  // Initiate repository.
  RepositoryPtr repo = _gitOps.createRepository(repoPath_);

  if (!_ctx.options.count("skip-forgetting"))
  {
    // Initiate walker.
    RevWalkPtr walker = _gitOps.createRevWalk(repo.get());
    git_revwalk_sorting(walker.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker.get());

    git_oid oid;
    int allCommits = 0;
    while (git_revwalk_next(&oid, walker.get()) == 0)
      ++allCommits;

    LOG(info) << "[competenceparser] Sampling " << allCommits << " commits in git repository.";

    // TODO: nice function to determine sample size
    int sampleSize = 1;//(int)std::sqrt((double)allCommits);
    LOG(info) << "[competenceparser] Sample size is " << sampleSize << ".";

    RevWalkPtr walker2 = _gitOps.createRevWalk(repo.get());
    git_revwalk_sorting(walker2.get(), GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
    git_revwalk_push_head(walker2.get());

    int commitCounter = 0;
    while (git_revwalk_next(&oid, walker2.get()) == 0)
    {
      ++commitCounter;
      if (commitCounter % sampleSize == 0)
      {
        CommitPtr commit = _gitOps.createCommit(repo.get(), oid);
        CommitSampler::CommitJob job(repoPath_, root_, oid, commit.get(), commitCounter);
        sampleCommits(job);
      }
    }
    persistSampleData();
  }
}

void CommitSampler::sampleCommits(CommitSampler::CommitJob& job_)
{
  RepositoryPtr repo = _gitOps.createRepository(job_._repoPath);

  // Retrieve parent of commit.
  CommitPtr parent = _gitOps.createParentCommit(job_._commit);

  if (!parent)
    return;

  // Get git tree of both commits.
  TreePtr commitTree = _gitOps.createTree(job_._commit);
  TreePtr parentTree = _gitOps.createTree(parent.get());

  if (!commitTree || !parentTree)
    return;

  // Calculate diff of trees.
  DiffPtr diff = _gitOps.createDiffTree(repo.get(), parentTree.get(), commitTree.get());

  // Loop through each delta.
  size_t num_deltas = git_diff_num_deltas(diff.get());
  if (num_deltas == 0)
    return;

  // Analyse every file that was affected by the commit.
  for (size_t j = 0; j < num_deltas; ++j)
  {
    const git_diff_delta *delta = git_diff_get_delta(diff.get(), j);
    git_diff_file diffFile = delta->new_file;
    model::FilePtr file = _ctx.srcMgr.getFile(job_._root + "/" + diffFile.path);
    if (!file)
      continue;

    auto iter = _commitSample.find(file);
    if (iter == _commitSample.end())
      _commitSample.insert({file, 1});
    else
      ++(iter->second);
  }
}

void CommitSampler::persistSampleData()
{
  for (const auto& pair : _commitSample)
  {
    util::OdbTransaction transaction(_ctx.db);
    transaction([&, this]
    {
      model::SampleData sample;
      sample.file = pair.first->id;
      sample.occurrences = pair.second;
      _ctx.db->persist(sample);
    });
  }
}
}
}