/*
 * gitcommit.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitcommit.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{
  
GitCommit::~GitCommit() {
  if (pInternal)
  {
    git_commit_free(pInternal);
  }
}

GitOid GitCommit::getId() const {
  GitOid rv(git_commit_id(pInternal)->id);
  return rv;  
}


//factory


GitCommit GitCommit::lookUp(GitRepository &repo, GitOid oid)
{
  git_commit *ret;
  git_oid x;
  oid._internal_fill(x);
  int error = git_commit_lookup(&ret, repo.getInternal(), &x);
  GitException::hadleError(error);
  GitCommit ret_object(ret);
  return std::move(ret_object);
}


//getters
const char* GitCommit::getMessage() const
{
  return git_commit_message(pInternal);
}
 
//note that libgit2 HEAD has a git_commit_summary function
//but I wanted to use a released version
//Change to official version should be considered after libgit2 v0.21 is released.
std::string GitCommit::getSummary() const
{
  std::string commitMsg(getMessage());
  auto pos = commitMsg.find('\n');
  if (std::string::npos == pos) {
    return commitMsg;
  } else {
    return commitMsg.substr(0, pos);
  }
}


const GitSignature GitCommit::getCommitter() const {
  const git_signature *sig = git_commit_committer(pInternal);
  return GitSignature(sig);
}

const GitSignature GitCommit::getAuthor() const {
  const git_signature *sig = git_commit_author(pInternal);
  return GitSignature(sig);
}

int64_t GitCommit::getTime() const
{
  int64_t ret = git_commit_time(pInternal);
  return ret;
}

int GitCommit::getTimeOffset() const
{
  int ret = git_commit_time_offset(pInternal);
  return ret;
}

  
  

//parent commit retrieval
size_t GitCommit::getParentCount() const
{
  size_t ret = git_commit_parentcount(pInternal);
  return ret;
}

GitCommit GitCommit::getParent(size_t index)
{
  git_commit *ret;
  git_commit_parent(&ret, pInternal, index);
  return std::move(GitCommit(ret));
}

GitOid GitCommit::getParentId(size_t index)
{
  const git_oid *ret = git_commit_parent_id(pInternal, index);
  return GitOid(ret->id);
}

//tree retrieval
GitOid GitCommit::getTreeId()
{
  GitOid ret(git_commit_tree_id(pInternal)->id);
  return ret;
}

GitTree GitCommit::getTree()
{
  git_tree *out;
  int error = git_commit_tree(&out, pInternal);
  GitException::hadleError(error);
  return std::move(GitTree(out));
}
  
} /* namespace parser */
} /* namespace cc */
