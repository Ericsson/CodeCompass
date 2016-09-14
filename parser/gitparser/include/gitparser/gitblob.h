/*
 * gitblob.h
 *
 *  Created on: Feb 24, 2014
 *      Author: cseri
 */

#ifndef GITBLOB_H
#define GITBLOB_H

#include <cstdint>
#include <string>

#include "gitoid.h"

struct git_blob;

namespace cc
{
namespace parser
{

class GitRepository;

/**
 * Represents a blob object in a Git repository.
 *
 * This class is a wrapper for libgit2's <tt>git_blob</tt>.
 */
class GitBlob
{
  git_blob *pInternal;

  explicit GitBlob(const GitBlob&) = delete;
  GitBlob& operator=(GitBlob) = delete;

  GitBlob(git_blob *pInternal) : pInternal(pInternal) { }

public:
  GitBlob(GitBlob&& o) : pInternal(o.pInternal)
  {
    o.pInternal = nullptr;
  }
  ~GitBlob();

  //factory
  static GitBlob lookUp(GitRepository &repo, GitOid oid);

  //getters
  GitOid getId() const;
  /**
   * Data associated with the blob. This is not null-terminated!
   */
  const char *getData() const;
  /**
   * The size (length) of the data of the blob.
   */
  size_t getDataSize() const;
  
  /**
   * Constructs an <tt>std::string</tt> object from the data of the blob.
   */
  std::string getDataAsString() const
  {
    return std::string(getData(), getDataSize());
  }

};


} /* namespace parser */
} /* namespace cc */

#endif // GITBLOB_H
