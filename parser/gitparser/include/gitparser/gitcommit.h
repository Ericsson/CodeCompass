/*
 * gitcommit.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <cstdint>

#include "gitoid.h"
#include "gittree.h"
#include "gitsignature.h"

struct git_commit;

namespace cc
{
namespace parser
{

class GitRepository;

/**
 * Represents a commit object in a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_commit</tt>.
 */
class GitCommit
{
  git_commit *pInternal;

  explicit GitCommit(const GitCommit&) = delete;
  GitCommit& operator=(GitCommit) = delete;

  GitCommit(git_commit *pInternal) : pInternal(pInternal) { };

public:
  GitCommit(GitCommit&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitCommit();

  //factory
  static GitCommit lookUp(GitRepository &repo, GitOid oid);

  //getters
  GitOid getId() const;
  const char *getMessageEncoding() const;
  const char *getMessage() const;
  std::string getSummary() const;
  int64_t getTime() const;
  int getTimeOffset() const;
  const GitSignature getCommitter() const;
  const GitSignature getAuthor() const;
  const char *getRawHeader() const;

  //parent commit retrieval
  size_t getParentCount() const;
  GitCommit getParent(size_t index);
  GitOid getParentId(size_t index);
  
  //tree retrieval
  GitOid getTreeId();
  GitTree getTree();

};


} /* namespace parser */
} /* namespace cc */

#endif // GITCOMMIT_H
