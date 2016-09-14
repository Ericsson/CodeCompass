/*
 * inheritance.h
 *
 *  Created on: May 7, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CXX_INHERITANCE_H_
#define MODEL_CXX_INHERITANCE_H_

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
 * From: Base class definition (instance of Type)
 * To: Derived class definition (instance of Type)
 */
#pragma db object
struct CppInheritance
{
  #pragma db id auto
  int id; /**< index of To of From's base classes */

  /**
   * Mangled name hash of derived type.
   */
  HashType derived;

  /**
   * Mangled name hash of base type.
   */
  HashType base;

  bool isVirtual = false;

  Visibility visibility; /**< private/protected/public */

#ifndef NO_INDICES
  #pragma db index member(derived)
  #pragma db index member(base)
#endif
};

typedef std::shared_ptr<CppInheritance> CppInheritancePtr;

} // model
} // cc


#endif /* MODEL_CXX_INHERITANCE_H_ */
