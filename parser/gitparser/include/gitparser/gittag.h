/*
 * gittag.h
 *
 *  Created on: Jun 25, 2014
 *      Author: cseri
 */

#ifndef GITTAG_H
#define GITTAG_H

#include <cstdint>
#include <string>

#include "gitoid.h"

#include "gitsignature.h"

struct git_tag;

namespace cc
{
namespace parser
{
  
class GitRepository;

/**
 * Represents a tag object in a Git repository.
 * 
 * This class is a wrapper for libgit2's <tt>git_tag</tt>.
 */
class GitTag
{
  git_tag *pInternal;

  explicit GitTag(const GitTag&) = delete;
  GitTag& operator=(GitTag) = delete;

  GitTag(git_tag *pInternal) : pInternal(pInternal) { }

public:
  GitTag(GitTag&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitTag();

  //factory
  static GitTag lookUp(GitRepository &repo, GitOid oid);

  //getters
  /**
   * The oid of the object
   * 
   * wrapper for <tt>git_tag_id</tt>
   */
  GitOid getId() const;
  
  /**
   * The message of the tag
   * 
   * wrapper for <tt>git_tag_message</tt>
   */
  const char* getMessage() const;

  /**
   * First line of the message of the tag
   */
  std::string getSummary() const;

  /**
   * The name of the tag
   * 
   * wrapper for <tt>git_tag_name</tt>
   */
  const char* getName() const;

  /**
   * The id of the object pointed by the tag
   * 
   * wrapper for <tt>git_tag_target_id</tt>
   */
  GitOid getTargetId() const;
  
  /**
   * The the signature of the tagger
   * 
   * wrapper for <tt>git_tag_tagger</tt>
   */
  GitSignature getTagger() const;

};


} /* namespace parser */
} /* namespace cc */

#endif // GITOBJECT_H
