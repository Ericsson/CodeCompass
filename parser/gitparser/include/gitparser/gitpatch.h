/*
 * gitpatch.h
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */



#ifndef GITPATCH_H
#define GITPATCH_H

#include <cstdint>
#include <string>

#include <memory>


struct git_patch;

namespace cc
{
namespace parser
{

class GitDiff;

/*
 *
 */
class GitPatch
{
  git_patch *pInternal;
  
  explicit GitPatch(const GitPatch&) = delete;
  GitPatch& operator=(GitPatch) = delete;

  GitPatch(git_patch *pInternal) : pInternal(pInternal) { };

public:
  GitPatch(GitPatch&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  };
  ~GitPatch();
  
  GitPatch(const GitDiff& o);

  


};


} /* namespace parser */
} /* namespace cc */

#endif // GITPATCH_H
