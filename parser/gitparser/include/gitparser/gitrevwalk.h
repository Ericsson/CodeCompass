/*
 * gitrevwalk.h
 *
 *  Created on: Jul 4, 2014
 *      Author: cseri
 */

#ifndef GITREVWALK_H
#define GITREVWALK_H

#include <cstdint>
#include <utility>

#include "gitoid.h"
#include "gitcommit.h"


struct git_revwalk;

namespace cc
{
namespace parser
{

class GitRepository;

/**
 * Represents a repository walk in a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_revwalk</tt>.
 */
class GitRevWalk
{
  git_revwalk *pInternal;

  explicit GitRevWalk(const GitRevWalk&) = delete;
  GitRevWalk& operator=(GitRevWalk) = delete;

  GitRevWalk(git_revwalk *pInternal) : pInternal(pInternal) { };

public:
  explicit GitRevWalk(GitRepository& repo);
  
  GitRevWalk(GitRevWalk&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitRevWalk();

  void push(GitOid oid);
  void pushGlob(std::string glob);
  void pushRef(std::string refName);
  
  void reset();
  
  std::pair<bool, GitOid> next();
};


} /* namespace parser */
} /* namespace cc */

#endif // GITREVWALK_H
