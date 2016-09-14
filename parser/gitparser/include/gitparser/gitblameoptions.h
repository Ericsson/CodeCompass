/*
 * gitblameoptions.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cseri
 */



#ifndef GITBLAMEOPTIONS_H
#define GITBLAMEOPTIONS_H

#include <cstdint>
#include <string>

#include <memory>

#include <gitparser/gitoid.h>


struct git_blame_options;

namespace cc
{
namespace parser
{

class GitRepository;

class GitBlame;

/*
 *
 */
class GitBlameOptions
{
  git_blame_options *pInternal;
  
  explicit GitBlameOptions(const GitBlameOptions&) = delete;
  GitBlameOptions& operator=(GitBlameOptions) = delete;
  
public:
  /**
   * Creates a clone operation with default options.
   */
  GitBlameOptions();
  
  ~GitBlameOptions();
  
  /**
   * Sets if the blame operation will create a bare repository
   */
  void setNewestCommit(GitOid newestCommit);
  
  /**
   * Sets if the blame operation will create a bare repository
   */
  void setOldestCommit(GitOid oldestCommit);
  
  /**
   * Sets the first line of the blame. Line number start with 1.
   * Default:1
   */
  void setMinLine(uint32_t minLine);
  
  /**
   * Sets the last line of the blame. Line number start with 1.
   * Default: whole file
   */
  void setMaxLine(uint32_t maxLine);

  
  //TODO there are more options for git_clone
  //the additional options should have functions as well
  
  
  friend class GitBlame;

};


} /* namespace parser */
} /* namespace cc */

#endif // GITBLAMEOPTIONS_H
