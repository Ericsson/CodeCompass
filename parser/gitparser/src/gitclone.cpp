/*
 * gitclone.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */

#include "gitparser/gitclone.h"

#include "git2.h"

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"

#include "gitinit.h"

namespace cc
{
namespace parser
{
  
GitClone::GitClone() : opts(new git_clone_options(GIT_CLONE_OPTIONS_INIT)) { }

GitClone::~GitClone() {
  delete opts;
  opts = nullptr;
}

void GitClone::setBare(bool b)
{
  opts->bare = b;
}

GitRepository GitClone::clone(
  const char *url,
  const char *localPath) const
{
  GitInit::init();
  
  git_repository *out;
  int error = git_clone(&out, url, localPath, opts);
  GitException::hadleError(error);
  return GitRepository(out);
}



} /* namespace parser */
} /* namespace cc */
