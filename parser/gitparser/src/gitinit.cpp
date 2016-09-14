/*
 * gitinit.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: cseri
 */

#include "gitinit.h"

#include "git2.h"

namespace cc
{
namespace parser
{
  
void GitInit::init() {
  static bool initialized = false;
  if (!initialized) {
    git_libgit2_init();
    initialized = true;
  }
}


} /* namespace parser */
} /* namespace cc */


