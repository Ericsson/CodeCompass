/*
 * gitrevwalk.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: cseri
 */


#include "gitparser/gitrevwalk.h"

#include "git2.h"

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"

namespace cc
{
namespace parser
{

GitRevWalk::GitRevWalk(GitRepository& repo)
{
  int error = git_revwalk_new(&pInternal, repo.getInternal());
  GitException::hadleError(error);
  
  git_revwalk_sorting(pInternal, /*GIT_SORT_TOPOLOGICAL | */GIT_SORT_TIME);
}

GitRevWalk::~GitRevWalk()
{
  if (pInternal) {
    git_revwalk_free(pInternal);
  }
}
  
void GitRevWalk::push(GitOid oid)
{
  git_oid x;
  oid._internal_fill(x);
  int error = git_revwalk_push(pInternal, &x);
  GitException::hadleError(error);
}

void GitRevWalk::pushGlob(std::string glob)
{
  int error = git_revwalk_push_glob(pInternal, glob.c_str());
  GitException::hadleError(error);
}

void GitRevWalk::pushRef(std::string refName)
{
  int error = git_revwalk_push_ref(pInternal, refName.c_str());
  GitException::hadleError(error);
}

void GitRevWalk::reset()
{
  git_revwalk_reset(pInternal);
}
  
std::pair<bool, GitOid> GitRevWalk::next()
{
  git_oid ret_oid;
  int error = git_revwalk_next(&ret_oid, pInternal);
  if (GIT_ITEROVER != error) {
    return std::make_pair(true, GitOid(ret_oid.id));
  } else {
    return std::make_pair(false, GitOid());
  }
}
  
} /* namespace parser */
} /* namespace cc */