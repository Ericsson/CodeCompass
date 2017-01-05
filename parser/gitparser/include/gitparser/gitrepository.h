/*
 * gitrepository.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITREPOSITORY_H
#define GITREPOSITORY_H

#include <memory>

struct git_repository;

namespace cc
{
namespace parser
{
  
class GitReference;

/**
 * Represents a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_repository</tt>.
 */
class GitRepository
{
  git_repository* pInternal;
  
  GitRepository(GitRepository& o) = delete;
  GitRepository& operator=(GitRepository& o) = delete;

public:
  GitRepository(GitRepository&& o);
  virtual ~GitRepository();

  GitRepository(git_repository *internal);

  git_repository* getInternal() { return pInternal; };

  /**
   * Wrapper method for git_repository_open
   * 
   * @return a GitRepository object representing the repository.
   */
  static GitRepository open(const char *path);

  //const ref would kill move semantics
  static GitRepository open(std::string path)
  {
    return std::move(open(path.c_str()));
  }
  
  /**
   * Wrapper method for git_repository_head
   * 
   * @return a unique_ptr to the GitReference object (to avoid circular
   * dependency)
   */
  std::unique_ptr<GitReference> head();
  
  /**
   * Wrapper method for git_repository_head_detached
   * 
   * @return true, if the head is detached, false if not.
   */
  bool isHeadDetached();
  
  
  /**
   * Wrapper method for git_repository_is_bare
   * 
   * @return true, if the repository is bare
   */
  bool isBare();
};

} /* namespace parser */
} /* namespace cc */

#endif // GITREPOSITORY_H
