/*
 * gitobject.h
 *
 *  Created on: Jun 25, 2014
 *      Author: cseri
 */

#ifndef GITOBJECT_H
#define GITOBJECT_H

#include <cstdint>
#include <string>

#include "gitoid.h"

struct git_object;

namespace cc
{
namespace parser
{
  
class GitRepository;

/**
 * Represents a general object in a Git repository.
 * 
 * This class is a wrapper for libgit2's <tt>git_object</tt>.
 * 
 * TODO right now this is only useable to get the type of the object
 * objects should be converted to concrete objects using inheritance
 * or (seems better) move constructors.
 */
class GitObject
{
  git_object *pInternal;

  explicit GitObject(const GitObject&) = delete;
  GitObject& operator=(GitObject) = delete;

  GitObject(git_object *pInternal) : pInternal(pInternal) { }

public:
  GitObject(GitObject&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitObject();

  /**
   * Basic type (loose or packed) of any Git object.
   * mirror of <tt>git_otype</tt>
   */
  enum ObjectType {
      GIT_OBJ_ANY = -2,       /**< Object can be any of the following */
      GIT_OBJ_BAD = -1,       /**< Object is invalid. */
      GIT_OBJ__EXT1 = 0,      /**< Reserved for future use. */
      GIT_OBJ_COMMIT = 1,     /**< A commit object. */
      GIT_OBJ_TREE = 2,       /**< A tree (directory listing) object. */
      GIT_OBJ_BLOB = 3,       /**< A file revision object. */
      GIT_OBJ_TAG = 4,        /**< An annotated tag object. */
      GIT_OBJ__EXT2 = 5,      /**< Reserved for future use. */
      GIT_OBJ_OFS_DELTA = 6,  /**< A delta, base is given by an offset. */
      GIT_OBJ_REF_DELTA = 7,  /**< A delta, base is given by object id. */
  };


  //factory
  static GitObject lookUp(GitRepository &repo, GitOid oid);

  //getters
  /**
   * The oid of the object
   * 
   * wrapper for <tt>git_object_id</tt>
   */
  GitOid getId() const;
  
  /**
   * The type of the object
   * 
   * wrapper for <tt>git_object_type</tt>
   */
  ObjectType getType() const;
  
};


} /* namespace parser */
} /* namespace cc */

#endif // GITOBJECT_H
