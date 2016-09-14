/*
 * gittag.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: cseri
 */

#include "gitparser/gittag.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{
  
GitTag::~GitTag() {
  if (pInternal)
  {
    git_tag_free(pInternal);
  }
}

GitOid GitTag::getId() const {
  GitOid rv(git_tag_id(pInternal)->id);
  return rv;  
}


//factory


GitTag GitTag::lookUp(GitRepository &repo, GitOid oid)
{
  git_tag *ret;
  git_oid x;
  oid._internal_fill(x);
  int error = git_tag_lookup(&ret, repo.getInternal(), &x);
  GitException::hadleError(error);
  GitTag ret_object(ret);
  return std::move(ret_object);
}


//getters

const char* GitTag::getMessage() const
{
  return git_tag_message(pInternal);
}

std::string GitTag::getSummary() const
{
  std::string commitMsg(getMessage());
  auto pos = commitMsg.find('\n');
  if (std::string::npos == pos) {
    return commitMsg;
  } else {
    return commitMsg.substr(0, pos);
  }
}

const char* GitTag::getName() const
{
  return git_tag_name(pInternal);
}

GitOid GitTag::getTargetId() const
{
  GitOid rv(git_tag_target_id(pInternal)->id);
  return rv;  
}
  
GitSignature GitTag::getTagger() const
{
  GitSignature rv(git_tag_tagger(pInternal));
  return rv;  
}
  
} /* namespace parser */
} /* namespace cc */
