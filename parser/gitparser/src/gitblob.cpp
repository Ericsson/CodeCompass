/*
 * gitblob.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: cseri
 */

#include "gitparser/gitblob.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{
  
GitBlob::~GitBlob() {
  if (pInternal)
  {
    git_blob_free(pInternal);
  }
}

GitOid GitBlob::getId() const {
  GitOid rv(git_blob_id(pInternal)->id);
  return rv;  
}


//factory


GitBlob GitBlob::lookUp(GitRepository &repo, GitOid oid)
{
  git_blob *ret;
  git_oid x;
  oid._internal_fill(x);
  int error = git_blob_lookup(&ret, repo.getInternal(), &x);
  GitException::hadleError(error);
  GitBlob ret_object(ret);
  return std::move(ret_object);
}


//getters
const char* GitBlob::getData() const
{
  return static_cast<const char*>(git_blob_rawcontent(pInternal));
}
  
size_t GitBlob::getDataSize() const
{
  return git_blob_rawsize(pInternal);
}
  
} /* namespace parser */
} /* namespace cc */
