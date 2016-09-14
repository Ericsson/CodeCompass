/*
 * gittreeentry.h
 *
 *  Created on: Feb 22, 2014
 *      Author: cseri
 */

#ifndef GITTREEENTRY_H
#define GITTREEENTRY_H

#include <cstdint>
#include <cassert>

#include "gitoid.h"
#include "gitsignature.h"

struct git_tree_entry;

namespace cc
{
namespace parser
{

class GitRepository;

/**
 * Represents an entry of a tree object in a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_tree_entry</tt>.
 */
class GitTreeEntry
{
  const git_tree_entry *pInternal;
  git_tree_entry *pInternalToBeFreed;

  explicit GitTreeEntry(const GitTreeEntry&) = delete;
  GitTreeEntry& operator=(GitTreeEntry) = delete;

public:
  GitTreeEntry(GitTreeEntry&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  };
  ~GitTreeEntry();

  GitTreeEntry(const git_tree_entry *pInternal) :
    pInternal(pInternal), pInternalToBeFreed(nullptr) { };

  GitTreeEntry(git_tree_entry *pInternal, bool pInternalMustBeFreed) :
    pInternal(pInternal), pInternalToBeFreed(pInternal) {
      assert(pInternalMustBeFreed);
    };

  
  enum FileMode {
    GIT_FILEMODE_NEW        = 0000000,
    GIT_FILEMODE_TREE       = 0040000,
    GIT_FILEMODE_BLOB       = 0100644,
    GIT_FILEMODE_BLOB_EXECUTABLE    = 0100755,
    GIT_FILEMODE_LINK       = 0120000,
    GIT_FILEMODE_COMMIT     = 0160000,
  };
    
  //getters
  FileMode getMode() const;
  const char *getName() const;
  GitOid getPointedId() const;
  
  
  std::string getNameAsString() const
  {
    return std::string(getName());
  }
};


} /* namespace parser */
} /* namespace cc */

#endif // GITTREEENTRY_H
