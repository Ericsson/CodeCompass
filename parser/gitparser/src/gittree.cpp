/*
 * gittree.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gittree.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{
  
GitTree::~GitTree() {
  if (pInternal)
  {
    git_tree_free(pInternal);
  }
}

GitOid GitTree::getId() const {
  GitOid rv(git_tree_id(pInternal)->id);
  return rv;  
}


//factory


GitTree GitTree::lookUp(GitRepository &repo, GitOid oid)
{
  git_tree *ret;
  git_oid x;
  oid._internal_fill(x);
  int error = git_tree_lookup(&ret, repo.getInternal(), &x);
  GitException::hadleError(error);
  GitTree ret_object(ret);
  return std::move(ret_object);
}


//getters



//tree entry retrieval
size_t GitTree::getEntryCount() const
{
  size_t ret = git_tree_entrycount(pInternal);
  return ret;
}

/**
 * Tree entries returned by this function are alive as
 * long as
 */
GitTreeEntry GitTree::getEntryByIndex(size_t index)
{
  const git_tree_entry *ret = git_tree_entry_byindex(pInternal, index);
  return GitTreeEntry(ret);
}


  
} /* namespace parser */
} /* namespace cc */
