/*
 * gitpatch.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */

#include "gitparser/gitpatch.h"

#include "git2.h"

#include "gitparser/gitdiff.h"

namespace cc
{
namespace parser
{
  
GitPatch::~GitPatch() {
  if (pInternal)
  {
    git_patch_free(pInternal);
  }
}




//factory




  
} /* namespace parser */
} /* namespace cc */