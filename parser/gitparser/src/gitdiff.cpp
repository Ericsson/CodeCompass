/*
 * gitdiff.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: cseri
 */

#include "gitparser/gitdiff.h"

#include "git2.h"

#include "gitparser/gitrepository.h"
#include "gitparser/gitexception.h"
#include "gitparser/gittree.h"
#include "gitparser/gitblob.h"

#include "util/streamlog.h"
using namespace cc::util;

namespace cc
{
namespace parser
{
  
GitDiff::~GitDiff() {
  if (pInternal)
  {
    git_diff_free(pInternal);
  }
}



//factory


GitDiff GitDiff::treeToTreeDiff(
  GitRepository &repo,
  const GitTree &old_tree,
  const GitTree &new_tree,
  int context_lines,
  bool force_binary,
  std::vector<std::string> in_pathspec)
{
  typedef char* pchar;
    
  git_diff *out;
  git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
  opts.context_lines = context_lines;
  if (force_binary) {
    opts.flags |= GIT_DIFF_FORCE_BINARY;
  }
  
  std::unique_ptr<std::vector<std::string>> pathspec;
  std::unique_ptr<pchar[]> pathspec_arr;
  if (!in_pathspec.empty()) {
    pathspec.reset(new std::vector<std::string>(in_pathspec));
    pathspec_arr.reset(new pchar[opts.pathspec.count]);
    
    opts.pathspec.count = pathspec->size();
    opts.pathspec.strings = pathspec_arr.get();
    for (size_t i = 0; i < opts.pathspec.count; ++i) {
      opts.pathspec.strings[i] = &(*pathspec)[i][0];
    }
  }
  int error = git_diff_tree_to_tree(
    &out,
    repo.getInternal(),
    old_tree.getInternal(),
    new_tree.getInternal(),
    &opts
  );
  GitException::hadleError(error);
  
  GitDiff ret_object(out, std::move(pathspec), std::move(pathspec_arr));
  return std::move(ret_object);
}



namespace {
  /**
   * callback to print diff without the actual changes
   */
  int gitdiff_str_print_cb_compact(
    const git_diff_delta* d,
    const git_diff_hunk* h,
    const git_diff_line* l,
    void *payload)
  {
    std::string& ret = *static_cast<std::string*>(payload);
    
    if (l->origin == GIT_DIFF_LINE_CONTEXT ||
      l->origin == GIT_DIFF_LINE_ADDITION ||
      l->origin == GIT_DIFF_LINE_DELETION ||
      l->origin == GIT_DIFF_LINE_CONTEXT ||
      l->origin == GIT_DIFF_LINE_CONTEXT_EOFNL ||
      l->origin == GIT_DIFF_LINE_ADD_EOFNL ||
      l->origin == GIT_DIFF_LINE_DEL_EOFNL ||
      l->origin == GIT_DIFF_LINE_BINARY
    ) {
      //nop
    } else {
      ret += std::string(l->content, l->content_len);
      //ret += "\n";
    }
    
    return 0;
  }
  
  int gitdiff_str_print_cb(
    const git_diff_delta* d,
    const git_diff_hunk* h,
    const git_diff_line* l,
    void *payload)
  {
    std::string& ret = *static_cast<std::string*>(payload);
    
    if (l->origin == GIT_DIFF_LINE_CONTEXT ||
      l->origin == GIT_DIFF_LINE_ADDITION ||
      l->origin == GIT_DIFF_LINE_DELETION) {
      ret += (char) l->origin;
    }
    
    ret += std::string(l->content, l->content_len);
    //ret += "\n";
    
    return 0;
  }
  
  int gitdiff_getDeltaList_file_cb(
    const git_diff_delta *delta,
    float /*progress*/,
    void* payload)
  {
    std::vector<GitDiff::GitDiffDelta>& ret =
      *static_cast<std::vector<GitDiff::GitDiffDelta>*>(payload);
      
    ret.emplace_back();
    GitDiff::GitDiffDelta &newElement = ret.back();

    switch (delta->status)
    {
      case GIT_DELTA_UNMODIFIED: newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_UNMODIFIED; break;
      case GIT_DELTA_ADDED:      newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_ADDED; break;
      case GIT_DELTA_DELETED:    newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_DELETED; break;
      case GIT_DELTA_MODIFIED:   newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_MODIFIED; break;
      case GIT_DELTA_RENAMED:    newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_RENAMED; break;
      case GIT_DELTA_COPIED:     newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_COPIED; break;
      case GIT_DELTA_IGNORED:    newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_IGNORED; break;
      case GIT_DELTA_UNTRACKED:  newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_UNTRACKED; break;
      case GIT_DELTA_TYPECHANGE: newElement.type = GitDiff::GitDiffDelta::GIT_DELTA_TYPECHANGE; break;
      default: throw std::runtime_error("Invalid status in delta.");
    }
    
    newElement.binary = delta->flags & GIT_DIFF_FLAG_BINARY;
    newElement.binary = delta->flags & GIT_DIFF_FLAG_NOT_BINARY;

    newElement.oldFile.fileMode = (GitTreeEntry::FileMode) delta->old_file.mode;
    newElement.oldFile.filePath = delta->old_file.path;
    newElement.oldFile.fileSize = delta->old_file.size;
    newElement.oldFile.fileOid = GitOid(delta->old_file.id.id);

    newElement.newFile.fileMode = (GitTreeEntry::FileMode) delta->new_file.mode;
    newElement.newFile.filePath = delta->new_file.path;
    newElement.newFile.fileSize = delta->new_file.size;
    newElement.newFile.fileOid = GitOid(delta->new_file.id.id);
    
    return 0;
  }
  
}



std::string GitDiff::compactStr()
{
  std::string ret;
  SLog(INFO) << "GitDiff::str pInternal: " << pInternal;
  int error = git_diff_print(
    pInternal,
    GIT_DIFF_FORMAT_PATCH,
    gitdiff_str_print_cb_compact,
    &ret
  );
  GitException::hadleError(error);
  
  SLog(INFO) << "GitDiff::str ready, length: " << ret.length();
  return ret;
}

std::string GitDiff::str()
{
  std::string ret;
  SLog(INFO) << "GitDiff::str pInternal: " << pInternal;
  int error = git_diff_print(
    pInternal,
    GIT_DIFF_FORMAT_PATCH,
    gitdiff_str_print_cb,
    &ret
  );
  GitException::hadleError(error);
  
  SLog(INFO) << "GitDiff::str ready, length: " << ret.length();
  return ret;
}

std::size_t GitDiff::getNumDeltas()
{
  return git_diff_num_deltas(pInternal);
}

std::vector<GitDiff::GitDiffDelta> GitDiff::getDeltaList()
{
  std::vector<GitDiffDelta> ret;
  
  int error = git_diff_foreach(
    pInternal,
    gitdiff_getDeltaList_file_cb,
    nullptr, 
    nullptr,
    nullptr,
    &ret
  );
  GitException::hadleError(error);

  return ret;
}


  
} /* namespace parser */
} /* namespace cc */
