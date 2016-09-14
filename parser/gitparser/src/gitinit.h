/*
 * gitinit.h
 *
 *  Created on: Nov 23, 2014
 *      Author: cseri
 */

#ifndef GITINIT_H
#define GITINIT_H



struct git_commit;

namespace cc
{
namespace parser
{

/**
 * Initializes libgit2.
 */
class GitInit
{
public:
  /**
  * Initializes libgit2.
  * Must be called before any other functions.
  * 
  * Called from the entry points of the library, e.g. GitRepository::lookup
  */
  static void init();
};


} /* namespace parser */
} /* namespace cc */

#endif // GITINIT_H
