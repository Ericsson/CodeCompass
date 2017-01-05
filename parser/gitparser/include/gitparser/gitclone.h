/*
 * gitclone.h
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */



#ifndef GITCLONE_H
#define GITCLONE_H

#include <cstdint>
#include <string>

#include <memory>


struct git_clone_options;

namespace cc
{
namespace parser
{

class GitRepository;

/*
 *
 */
class GitClone
{
  git_clone_options *opts;
  
  explicit GitClone(const GitClone&) = delete;
  GitClone& operator=(GitClone) = delete;

public:
  /**
   * Creates a clone operation with default options.
   */
  GitClone();
  
  ~GitClone();
  
  /**
   * Sets if next clone operation will create a bare repository
   */
  void setBare(bool b);
  
  //TODO there are more options for git_clone
  //the additional options should have functions as well
  
  
  /**
   * Clones a repostitory using the previously set options
   */
  GitRepository clone(
    const char *url,
    const char *localPath) const;


};


} /* namespace parser */
} /* namespace cc */

#endif // GITCLONE_H
