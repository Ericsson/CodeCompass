/*
 * gitreference.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitreference.h"

#include "git2.h"

#include "gitparser/githexoid.h"

#include "gitparser/gitexception.h"

namespace cc
{
namespace parser
{

  
  
  
  
GitReference::~GitReference()
{
  if (pInternal) {
    git_reference_free(pInternal);
  }
}

GitReference::Type GitReference::getType()
{
  git_ref_t type = git_reference_type(pInternal);
  switch (type)
  {
    case GIT_REF_INVALID:   return Type::GIT_REF_INVALID;
    case GIT_REF_OID:       return Type::GIT_REF_OID;
    case GIT_REF_SYMBOLIC:  return Type::GIT_REF_SYMBOLIC;
    case GIT_REF_LISTALL:   return Type::GIT_REF_LISTALL;
  }
  throw std::logic_error("Impossible reference type");
}
  
const char *GitReference::getSymbolicTarget()
{
  return git_reference_symbolic_target(pInternal);
}

GitOid GitReference::getTarget()
{
  const git_oid* oid = git_reference_target(pInternal);
  if (nullptr == oid) {
    throw std::runtime_error("called getTarget on non-normal reference");
  }
  
  GitOid ret(oid->id);
  return ret;
}

const char *GitReference::getName()
{
  return git_reference_name(pInternal);
}
    
  
GitOid GitReference::nameToId(GitRepository& repo, const char* name)
{
  git_oid ret;
  int error = git_reference_name_to_id(&ret, repo.getInternal(), name);
  if (error) {
    try {
      // maybe the name is already an OID
      return GitHexOid(name).toOid();
    } catch (std::runtime_error&)
    {
      throw GitException(error);
    }
  }
  return GitOid(ret.id);
}

std::vector<std::string> GitReference::getList(GitRepository& repo)
{
  git_strarray saRepoList;
  int error = git_reference_list(&saRepoList, repo.getInternal());
  if (error) {
    throw GitException(error);
  }
  
  std::vector<std::string> ret;
  ret.reserve(saRepoList.count);
  for (size_t i = 0; i < saRepoList.count; ++i)
  {
    ret.push_back(saRepoList.strings[i]);
  }

  git_strarray_free(&saRepoList);
  
  return std::move(ret);
}

std::vector<std::string> GitReference::getBranches(GitRepository& repo)
{
  std::vector<std::string> ret;
  
  git_branch_iterator *it;
  int error = git_branch_iterator_new(&it, repo.getInternal(), GIT_BRANCH_ALL);
  if (error) {
    throw GitException(error);
  }
  
  git_reference *ref;
  git_branch_t branch_type;
  while(git_branch_next(&ref, &branch_type, it) == 0)
  {
    const char *branch_name;
    error = git_branch_name(&branch_name, ref);    
    if (error) {
      throw GitException(error);
    }
    
    if(branch_type == GIT_BRANCH_REMOTE)
      ret.push_back("refs/remotes/" + std::string(branch_name));
    else if(git_branch_is_head(ref) == 1)    
      ret.push_back("refs/heads/" + std::string(branch_name));
  }
  git_reference_free(ref);
  git_branch_iterator_free(it);
  
  return ret;
}

std::vector<std::string> GitReference::getTags(GitRepository& repo)
{
  git_strarray saRepoList;
  int error = git_tag_list(&saRepoList, repo.getInternal());
  if (error) {
    throw GitException(error);
  }
  
  std::vector<std::string> ret;
  ret.reserve(saRepoList.count);
  for (size_t i = 0; i < saRepoList.count; ++i)
  {
    ret.push_back("refs/tags/" + std::string(saRepoList.strings[i]));
  }

  git_strarray_free(&saRepoList);
  
  return ret;
}


} /* namespace parser */
} /* namespace cc */