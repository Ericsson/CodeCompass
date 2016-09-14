/*
 * cpprelation.h
 *
 *  Created on: Oct 16, 2013
 *      Author: ezoltbo
 */

#ifndef CPPRELATION_H_
#define CPPRELATION_H_

#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{


#pragma db object
struct CppRelation
{
  enum class Kind
  {
    Override = 0,
    Alias,
    Assign,
    DeclContext
  };

  #pragma db id auto
  int id; /**< index of To of From's base classes */

  unsigned long long lhs;

  unsigned long long rhs;

  Kind kind;

#ifndef NO_INDICES
  #pragma db index member(lhs)
  #pragma db index member(rhs)
#endif
};

typedef odb::lazy_shared_ptr<CppRelation> CppRelationPtr;

} // model
} // cc


#endif /* CPPRELATION_H_ */
