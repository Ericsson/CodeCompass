/*
 * githexoid.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITHEXOID_H
#define GITHEXOID_H

#include <iosfwd>
#include <utility>
#include <cstdlib>
#include <string>

#include "gitoid.h"

namespace cc
{
namespace parser
{

/**
 * Represents an object id in hexadecimal format
 *
 * A GitOid can be converted into GitHexOid
 */
class GitHexOid
{
  typedef char internal_hex_oid_t[GIT_OID_HEXSZ + 1];

  internal_hex_oid_t hexOid;

public:
  GitHexOid(GitOid);

  GitHexOid(const char *);

  GitHexOid(const std::string &s) : GitHexOid(s.c_str()) { }

  const char *c_str() const
  {
    return hexOid;
  }
  
  std::string str() const
  {
    return std::string(c_str());
  }
  
  void copyToArray(internal_hex_oid_t &param) const
  {
    for (int i = 0; i < GIT_OID_HEXSZ + 1; ++i)
    {
      param[i] = hexOid[i];
    }
  }
  
  GitOid toOid() const
  {
    GitOid oid;
    for (int i = 0; i < GIT_OID_RAWSZ; ++i)
    {
      char ch[3];
      ch[0] = hexOid[2*i];
      ch[1] = hexOid[2*i+1];
      ch[2] = 0;
      long l = std::strtol(ch, nullptr, 16);
      oid._internal_getOid()[i] = static_cast<unsigned char>(l);
    }
    return std::move(oid);
  }

};

std::ostream& operator<<(std::ostream &o, GitHexOid gho);

} /* namespace parser */
} /* namespace cc */


#endif // GITHEXOID_H
