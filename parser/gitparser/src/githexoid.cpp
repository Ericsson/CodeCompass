/*
 * githexoid.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#include "gitparser/githexoid.h"

#include <iostream>
#include <cstring>
#include <stdexcept>

#include "git2.h"

namespace cc
{
namespace parser
{
  
std::ostream& operator<<(std::ostream &o, GitHexOid gho)
{
  return o << gho.c_str();
}

GitHexOid::GitHexOid(GitOid oid)
{
  git_oid oid_;
  oid._internal_fill(oid_);

  static_assert(sizeof(hexOid) == GIT_OID_HEXSZ + 1, "cannot store GitHexOid.");
  git_oid_tostr(hexOid, sizeof(hexOid), &oid_);
}

GitHexOid::GitHexOid(const char *input)
{
  if (!input) {
    throw std::runtime_error("The string used to construct GitHexOid was a nullpointer");
  }

  auto len = strlen(input);
  if (len != GIT_OID_HEXSZ) {
    throw std::runtime_error("The string used to construct GitHexOid has invalid length: " + std::to_string(len));
  }
  
  for (int i = 0; i < GIT_OID_HEXSZ; ++i)
  {
    if (!(
        ('0' <= input[i] && input[i] <= '9') ||
        ('a' <= input[i] && input[i] <= 'f') ||
        ('A' <= input[i] && input[i] <= 'F')
        )
    ) {
      throw std::runtime_error("The string used to construct GitHexOid contains invalid character");
    }
    
    hexOid[i] = input[i];
  }
  hexOid[GIT_OID_HEXSZ] = 0;
}


} /* namespace parser */
} /* namespace cc */


