/*
 * gitoid.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/gitoid.h"
#include "gitparser/githexoid.h"

#include "git2.h"


namespace cc
{
namespace parser
{

void GitOid::_internal_fill(git_oid& x) const
{
  //not to nice but works
  for (std::size_t i = 0; i < GIT_OID_RAWSZ; ++i) {
    x.id[i] = oid[i];
  }
}
  
std::ostream& operator<<(std::ostream& o, const GitOid& oid)
{
  GitHexOid h(oid);
  return o << h;
}

} /* namespace parser */
} /* namespace cc */

