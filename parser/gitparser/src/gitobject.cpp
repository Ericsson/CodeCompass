/*
 * gitobject.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: cseri
 */

#include "gitparser/gitobject.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{
  
GitObject::~GitObject() {
  if (pInternal)
  {
    git_object_free(pInternal);
  }
}

GitOid GitObject::getId() const {
  GitOid rv(git_object_id(pInternal)->id);
  return rv;  
}


//factory


GitObject GitObject::lookUp(GitRepository &repo, GitOid oid)
{
  git_object *ret;
  git_oid x;
  oid._internal_fill(x);
  int error = git_object_lookup(&ret, repo.getInternal(), &x, ::GIT_OBJ_ANY);
  GitException::hadleError(error);
  GitObject ret_object(ret);
  return std::move(ret_object);
}


//getters
GitObject::ObjectType GitObject::getType() const
{
  return static_cast<ObjectType>(git_object_type(pInternal));
}
  
} /* namespace parser */
} /* namespace cc */
