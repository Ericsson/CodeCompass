/*
 * gitdiff.h
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */



#ifndef GITDIFF_H
#define GITDIFF_H

#include <cstdint>
#include <string>
#include <vector>

#include <memory>
#include "gitparser/gittreeentry.h"


struct git_diff;

namespace cc
{
namespace parser
{

class GitRepository;
class GitTree;

/**
 * Represents a diff
 * 
 * This class is a wrapper for libgit2's <tt>git_diff</tt>.
 */
class GitDiff
{
  git_diff *pInternal;
  
  std::unique_ptr<std::vector<std::string>> _pathspec;
  std::unique_ptr<char*[]> _pathspec_arr;
  
  explicit GitDiff(const GitDiff&) = delete;
  GitDiff& operator=(GitDiff) = delete;

  GitDiff(git_diff *pInternal,
          std::unique_ptr<std::vector<std::string>> pathspec,
          std::unique_ptr<char*[]> pathspec_arr) :
    pInternal(pInternal),
    _pathspec(std::move(pathspec)),
    _pathspec_arr(std::move(pathspec_arr))
    { }
  

public:
  GitDiff() : pInternal(nullptr) { }
  GitDiff(GitDiff&& o) :
    pInternal(o.pInternal),
    _pathspec(std::move(o._pathspec)),
    _pathspec_arr(std::move(o._pathspec_arr))
  {
    o.pInternal = nullptr;
  }

  git_diff *getInternal()
  {
   return pInternal;
  }
  ~GitDiff();
  
  
  
  /**
   * Constructs a diff string from this object without the actual changes
   * (basically file list)
   */
  std::string compactStr();

  /**
   * Constructs a diff string from this object
   */
  std::string str();

  /**
   * Returns the number of deltas in the diff.
   * 
   * Wrapper for <tt>git_diff_num_deltas</tt>
   */
  std::size_t getNumDeltas();

  /**
   * Struct representing the old or the new file in a delta
   */
  struct GitDiffDeltaFile
  {
    std::string filePath;
    std::size_t fileSize;
    GitOid fileOid;
    GitTreeEntry::FileMode fileMode;
  };

  /**
   * A struct representing changes to one entry.
   * 
   * This class is corresponding to libgit2's <tt>git_diff_delta</tt>.
   */
  struct GitDiffDelta
  {
    enum DeltaType_t {
        GIT_DELTA_UNMODIFIED = 0,
        GIT_DELTA_ADDED = 1,
        GIT_DELTA_DELETED = 2,
        GIT_DELTA_MODIFIED = 3,
        GIT_DELTA_RENAMED = 4,
        GIT_DELTA_COPIED = 5,
        GIT_DELTA_IGNORED = 6,
        GIT_DELTA_UNTRACKED = 7,
        GIT_DELTA_TYPECHANGE = 8,
    };

    DeltaType_t type;
    
    bool binary;
    bool nonBinary;

    GitDiffDeltaFile oldFile;
    GitDiffDeltaFile newFile;
  };
  
  /**
   * Consturcts list of changed files with their old a new content
   */
  std::vector<GitDiffDelta> getDeltaList();
  

  /**
   * Construct a diff of two trees
   * 
   * TODO more options
   */
  static GitDiff treeToTreeDiff(
    GitRepository &repo,
    const GitTree &old_tree,
    const GitTree &new_tree,
    int context_lines,
    bool force_binary,
    std::vector<std::string> pathspec = std::vector<std::string>());

};


} /* namespace parser */
} /* namespace cc */

#endif // GITDIFF_H
