/*
 * cppfriendship.h
 *
 *  Created on: Jun 10, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CXX_CPPFRIENDSHIP_H_
#define MODEL_CXX_CPPFRIENDSHIP_H_

#include <string>
#include <memory>
#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/fileloc.h>
#include <model/cxx/cpptype.h>

namespace cc
{
namespace model
{


/**
 * This relation represents inheritenceship between user-defined types.
 *
 * target: a class definition
 * theFriend: the friend of the target
 */
#pragma db object
struct CppFriendship
{
  #pragma db id auto
  int id; /**< index of To of From's base classes */

  unsigned long long target;

  unsigned long long theFriend;

#ifndef NO_INDICES
  #pragma db index member(target)
  #pragma db index member(theFriend)
#endif
};

typedef std::shared_ptr<CppFriendship> CppFriendshipPtr;

} // model
} // cc

#endif /* CPPFRIENDSHIP_H_ */
