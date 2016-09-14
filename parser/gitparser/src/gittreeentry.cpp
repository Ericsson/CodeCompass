/*
 * gittreeentry.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gittreeentry.h"

#include "git2.h"

#include "gitparser/gitrepository.h"

namespace cc
{
namespace parser
{
  
GitTreeEntry::~GitTreeEntry() {
  if (pInternalToBeFreed)
  {
    git_tree_entry_free(pInternalToBeFreed);
  }
}




//getters


GitTreeEntry::FileMode GitTreeEntry::getMode() const
{
  return static_cast<GitTreeEntry::FileMode>(git_tree_entry_filemode_raw(pInternal));
}

const char* GitTreeEntry::getName() const
{
  return git_tree_entry_name(pInternal);
}
  
GitOid GitTreeEntry::getPointedId() const {
  //const git_oid* git_tree_entry

  return GitOid(git_tree_entry_id(pInternal)->id);
  
}



  
} /* namespace parser */
} /* namespace cc */
