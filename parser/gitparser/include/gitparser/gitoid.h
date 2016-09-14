/*
 * gitoid.h
 *
 *  Created on: Feb 16, 2014
 *      Author: cseri
 */

#ifndef GITOID_H
#define GITOID_H

#include <cstddef>
#include <iosfwd>
#include <functional> //for std::hash

struct git_oid;

//necessary defines from libgit2
#define GIT_OID_RAWSZ 20
#define GIT_OID_HEXSZ (GIT_OID_RAWSZ * 2)


namespace cc
{
namespace parser
{

/**
 * Class to represent a Git object id.
 *
 * This class is a lightweight header-only wrapper around an object id.
 */
class GitOid
{
  typedef unsigned char internal_oid_t[GIT_OID_RAWSZ];
  internal_oid_t oid;

public:
  GitOid()
  {
    for (std::size_t i = 0; i < GIT_OID_RAWSZ; ++i) {
      oid[i] = 0;
    }
  }

  GitOid(const internal_oid_t o) {
    for (std::size_t i = 0; i < GIT_OID_RAWSZ; ++i) {
      oid[i] = o[i];
    }
  }
  
  internal_oid_t &_internal_getOid()
  {
    return oid;
  }

  const internal_oid_t &_internal_getOid() const
  {
    return oid;
  }
  
  void _internal_fill(git_oid& x) const;
  
};

std::ostream& operator<<(std::ostream& o, const GitOid& oid);

inline bool operator==(const GitOid& o1, const GitOid& o2)
{
  for (std::size_t i = 0; i < GIT_OID_RAWSZ; ++i) {
    if (o1._internal_getOid()[i] != o2._internal_getOid()[i]) return false;
  }
  return true;
}

inline bool operator!=(const GitOid& o1, const GitOid& o2)
{
  return !(o1 == o2);
}

} /* namespace parser */
} /* namespace cc */


namespace std
{
  template<>
  struct hash<cc::parser::GitOid>
  {
    typedef std::size_t result_type;

    result_type operator()(const cc::parser::GitOid& s) const
    {
      static_assert(sizeof(size_t) < GIT_OID_RAWSZ, "size_t is too big");

      //first size_t bytes should give enough diversity
      return *(size_t*)(&s._internal_getOid());
    }
  };
} /* namespace std */

#endif // GITOID_H
