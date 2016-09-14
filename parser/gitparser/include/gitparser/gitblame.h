/*
 * gitblame.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cseri
 */



#ifndef GITBLAME_H
#define GITBLAME_H

#include <cstdint>
#include <string>

#include <memory>

#include "gitblameoptions.h"
#include "gitblamehunk.h"


struct git_blame;

namespace cc
{
namespace parser
{

class GitRepository;

/*
 *
 */
class GitBlame
{
  git_blame *pInternal;
  
  explicit GitBlame(const GitBlame&) = delete;
  GitBlame& operator=(const GitBlame&) = delete;

  GitBlame(git_blame *pInternal) : pInternal(pInternal) { }

public:

  GitBlame(GitBlame&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  GitBlame& operator=(GitBlame&& o)
  {
    this->~GitBlame();
    pInternal = o.pInternal;
    o.pInternal = nullptr;
    
    return *this;
  };

  ~GitBlame();
  
  /**
   * Retrieves the number of hunks
   * 
   * Wrapper for git_blame_get_hunk_count
   */
  uint32_t getHunkCount();
  
  /**
   * Retrieves the ith hunk
   * 
   * Wrapper for git_blame_get_hunk_byindex
   */
  GitBlameHunk getHunkByIndex(uint32_t index);
  
  /**
   * Retrieves the hunk in a line
   * 
   * Wrapper for git_blame_get_hunk_byline
   */
  GitBlameHunk getHunkByLine(uint32_t lineno);
  
  /**
   * Updates blame with (in-memory) local modifications
   * 
   * Wrapper for git_blame_buffer
   */
  GitBlame blameBuffer(const std::string &buf);

  
  /**
   * Creates a blame for a file
   * 
   * Wrapper for git_blame_file
   */
  static GitBlame file(
    GitRepository &repo,
    const char* path,
    const GitBlameOptions &opts
  );


};


} /* namespace parser */
} /* namespace cc */

#endif // GITBLAME_H
