#ifndef CC_PARSER_COMMITSAMPLER_H
#define CC_PARSER_COMMITSAMPLER_H

#include "competenceparser/gitoperations.h"

#include <model/file.h>
#include <parser/parsercontext.h>
#include <util/parserutil.h>
#include <util/threadpool.h>

namespace cc
{
namespace parser
{

class CommitSampler
{
public:
  CommitSampler(ParserContext& ctx_);

  struct CommitJob
  {
    boost::filesystem::path& _repoPath;
    const std::string& _root;
    git_oid _oid;
    git_commit* _commit;
    int _commitCounter;

    CommitJob(boost::filesystem::path& repoPath_,
              const std::string& root_,
              git_oid oid_,
              git_commit* commit_,
              int commitCounter_)
      : _repoPath(repoPath_), _root(root_), _oid(oid_),
        _commit(commit_), _commitCounter(commitCounter_) {}
  };

  void commitSampling(
    const std::string& root_,
    boost::filesystem::path& repoPath_);

private:
  void sampleCommits(CommitJob& job_);
  void persistSampleData();

  ParserContext& _ctx;
  static GitOperations _gitOps;
  std::map<model::FilePtr, int> _commitSample;
};
}
}

#endif // CC_PARSER_COMMITSAMPLER_H
