/*
 * gitblamehunk.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cseri
 */



#ifndef GITBLAMEHUNK_H
#define GITBLAMEHUNK_H

#include <cstdint>
#include <string>

#include <memory>

#include <gitparser/gitoid.h>
#include <gitparser/gitsignature.h>


struct git_blame_options;

namespace cc
{
namespace parser
{

/**
 * Mirroring of git_blame_hunk. The documentation was copied from there.
 * 
 * GitBlame returns these objects.
 */
struct GitBlameHunk
{

  /**
   * the number of lines in this hunk
   */
  uint16_t lines_in_hunk;
  
  /**
   * the OID of the commit where this line was last changed
   */
  GitOid final_commit_id;
  
  /**
   * the 1-based line number where this hunk begins, in the final version of
   * the file
   */
  uint16_t final_start_line_number;
  
  GitSignature final_signature;
  
  /**
   * the OID of the commit where this hunk was found. This will usually be
   * the same as final_commit_id, except when
   * GIT_BLAME_TRACK_COPIES_ANY_COMMIT_COPIES has been specified.
   * 
   * TODO implement blame flags
   */
  GitOid orig_commit_id;
  
  /**
   * the path to the file where this hunk originated, as of the commit
   * specified by orig_commit_id.
   */
  const char *orig_path;
  
  /**
   * the 1-based line number where this hunk begins in the file named by
   * orig_path in the commit specified by orig_commit_id.
   */
  uint16_t orig_start_line_number;
  
  GitSignature orig_signature;

  /**
   * 1 iff the hunk has been tracked to a boundary commit (the root, or the
   * commit specified in git_blame_options.oldest_commit)
   */
  bool boundary;

};


} /* namespace parser */
} /* namespace cc */

#endif // GITBLAMEHUNK_H
