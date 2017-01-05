/*
 * gitblameoptions.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: cseri
 */

#include "gitparser/gitblameoptions.h"

#include "git2.h"

namespace cc
{
namespace parser
{

GitBlameOptions::GitBlameOptions() : pInternal(new git_blame_options) {
  *pInternal = GIT_BLAME_OPTIONS_INIT;
}

GitBlameOptions::~GitBlameOptions() {
  if (pInternal)
  {
    delete(pInternal);
  }
}

void GitBlameOptions::setNewestCommit(GitOid newestCommit)
{
  newestCommit._internal_fill(pInternal->newest_commit);
}
  
void GitBlameOptions::setOldestCommit(GitOid oldestCommit)
{
  oldestCommit._internal_fill(pInternal->oldest_commit);
}
  
void GitBlameOptions::setMinLine(uint32_t minLine)
{
  pInternal->min_line = minLine;
}
  
void GitBlameOptions::setMaxLine(uint32_t maxLine)
{
  pInternal->max_line = maxLine;
}




//factory


  
} /* namespace parser */
} /* namespace cc */
