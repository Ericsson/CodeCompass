/*
 * gitblame.cpp
 *
 *  Created on: Jun 12, 2014
 *      Author: cseri
 */

#include "gitparser/gitblame.h"

#include "git2.h"

#include "gitparser/gitexception.h"
#include "gitparser/gitrepository.h"

namespace cc
{
namespace parser
{
  
GitBlame::~GitBlame() {
  if (pInternal)
  {
    git_blame_free(pInternal);
  }
}



//factory

GitBlame GitBlame::file(
  GitRepository &repo,
  const char* path,
  const GitBlameOptions &blameOpts)
{
  git_blame *ret;
  int error = git_blame_file(
    &ret,
    repo.getInternal(),
    path,
    blameOpts.pInternal
  );
  GitException::hadleError(error);
  GitBlame ret_object(ret);
  return std::move(ret_object);
}


//getters

uint32_t GitBlame::getHunkCount()
{
  return git_blame_get_hunk_count(pInternal);
}

namespace {
  GitBlameHunk convertHunk(const git_blame_hunk* hunk)
  {
    GitBlameHunk ret;
    ret.lines_in_hunk = hunk->lines_in_hunk;
    ret.final_commit_id = GitOid(hunk->final_commit_id.id);
    ret.final_start_line_number = hunk->final_start_line_number;
    if (hunk->final_signature) {
      ret.final_signature = GitSignature(hunk->final_signature);
    }
    ret.orig_commit_id = GitOid(hunk->orig_commit_id.id);
    ret.orig_path = hunk->orig_path;
    ret.orig_start_line_number = hunk->orig_start_line_number;
    if (hunk->orig_signature) {
      ret.orig_signature = GitSignature(hunk->orig_signature);
    }
    ret.boundary = hunk->boundary;
    return ret;
  }
}

GitBlameHunk GitBlame::getHunkByIndex(uint32_t index)
{
  const git_blame_hunk* hunk = git_blame_get_hunk_byindex(pInternal, index);
  return convertHunk(hunk);
}
  
GitBlameHunk GitBlame::getHunkByLine(uint32_t lineno)
{
  const git_blame_hunk* hunk = git_blame_get_hunk_byindex(pInternal, lineno);
  return convertHunk(hunk);
}

GitBlame GitBlame::blameBuffer(const std::string &buf)
{
  git_blame *ret;
  int error = git_blame_buffer(
    &ret,
    pInternal,
    buf.data(),
    buf.size()
  );
  GitException::hadleError(error);
  GitBlame ret_object(ret);
  return std::move(ret_object);
}
  
} /* namespace parser */
} /* namespace cc */
