/*
 * gitreference.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITREFERENCE_H
#define GITREFERENCE_H

#include <vector>
#include <string>

#include "gitoid.h"
#include "gitrepository.h"

struct git_reference;

namespace cc
{
namespace parser
{
  
/**
 * Representing a Git reference in the repository.
 * 
 * This class is a wrapper for libgit2's <tt>git_reference</tt>.
 */
class GitReference
{
  git_reference *pInternal;

  explicit GitReference(const GitReference&) = delete;
  GitReference& operator=(GitReference) = delete;

public:
  GitReference(GitReference&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitReference();
  
  GitReference(git_reference *pInternal) : pInternal(pInternal) { };

  /**
   * mirror libgit2's git_ref_t
   * 
   * (TODO might consider subclasses)
   */
  enum Type {
    GIT_REF_INVALID = 0, /** Invalid reference */
    GIT_REF_OID = 1, /** A reference which points at an object id */
    GIT_REF_SYMBOLIC = 2, /** A reference which points at another reference */
    GIT_REF_LISTALL = GIT_REF_OID|GIT_REF_SYMBOLIC,
  } ;

  /**
   * returns the type of the reference
   * 
   * wrapper for git_reference_type
   */
  Type getType();
  
  /**
   * returns the target of a symbolic reference
   * 
   * wrapper for git_reference_symbolic_target
   * 
   * @return nullptr if not a symbolic reference
   */
  const char *getSymbolicTarget();
  
  /**
   * returns the target of a reference
   * 
   * wrapper for git_reference_target
   * 
   * @throws runtime_error if not an oid reference
   */
  GitOid getTarget();
  
  /**
   * returns the name of a reference
   * 
   * wrapper for git_reference_name
   * 
   * @throws runtime_error if not a oid reference
   */
  const char *getName();

  
  static GitOid nameToId(GitRepository& repo, const char* name);
  
  static std::vector<std::string> getList(GitRepository& repo);
  
  static std::vector<std::string> getBranches(GitRepository& repo);
  
  static std::vector<std::string> getTags(GitRepository& repo);
};

  
} /* namespace parser */
} /* namespace cc */

#endif // GITREFERENCE_H
