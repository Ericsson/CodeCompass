/*
 * gitrepository.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitrepository.h"

#include <utility>
#include "git2.h"

#include "gitparser/gitexception.h"
#include "gitparser/gitreference.h"

#include "gitinit.h"

namespace cc
{
namespace parser
{

GitRepository::GitRepository(git_repository *internal) : pInternal(internal) { }

GitRepository::GitRepository(GitRepository &&o)
{
  pInternal = o.pInternal;
  o.pInternal = nullptr;
}

GitRepository::~GitRepository()
{
  if (pInternal) {
    git_repository_free(pInternal);
  }
}

GitRepository GitRepository::open(const char *path)
{
  GitInit::init();
  
  git_repository *ret;
  
  int error = git_repository_open(&ret, path);
  GitException::hadleError(error);

  GitRepository ret_obj(ret);
  return std::move(ret_obj);
}

std::unique_ptr<GitReference> GitRepository::head()
{
  git_reference *ref;

  int error = git_repository_head(&ref, pInternal);
  GitException::hadleError(error);
  
  std::unique_ptr<GitReference> ret(new GitReference(ref));
  return std::move(ret);
}

bool GitRepository::isHeadDetached()
{
  int ret = git_repository_head_detached(pInternal);
  if (0 == ret) return false;
  if (1 == ret) return true;
  GitException::hadleError(ret);
  
  throw std::logic_error("impossible code flow");
}

bool GitRepository::isBare()
{
  return git_repository_is_bare(pInternal);
}

} /* namespace parser */
} /* namespace cc */
