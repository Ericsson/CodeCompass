/*
 * gittree.h
 *
 *  Created on: Feb 22, 2014
 *      Author: cseri
 */

#ifndef GITTREE_H
#define GITTREE_H

#include <cstdint>

#include "gitoid.h"
#include "gittreeentry.h"

struct git_tree;

namespace cc
{
namespace parser
{

class GitRepository;

/**
 * Represents a tree object in a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_tree</tt>.
 */
class GitTree
{
  git_tree *pInternal;

  explicit GitTree(const GitTree&) = delete;
  GitTree& operator=(const GitTree&) = delete;

public:
  GitTree() : pInternal(nullptr) { };
  
  GitTree(git_tree *pInternal) : pInternal(pInternal) { };

  GitTree(GitTree&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  };
  GitTree& operator=(GitTree&& o)
  {
    this->~GitTree();
    pInternal = o.pInternal;
    o.pInternal = nullptr;
    
    return *this;
  };
  ~GitTree();

  git_tree *getInternal() const
  {
    return pInternal;
  }

  //factory
  static GitTree lookUp(GitRepository &repo, GitOid oid);

  //getters
  GitOid getId() const;
  const char *getMessageEncoding() const;

  //entry commit retrieval
  size_t getEntryCount() const;
  GitTreeEntry getEntryByIndex(size_t index);
  
};


} /* namespace parser */
} /* namespace cc */

#endif // GITTREE_H
