/*
 * cppenum.h
 *
 *  Created on: Sep 20, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CPPTYPEDEF_H_
#define MODEL_CPPTYPEDEF_H_

#include "cppentity.h"
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

/**
 * This class represents a typedef in c++
 */
#pragma db object
struct CppTypedef : CppTypedEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }
};

typedef std::shared_ptr<CppTypedef> CppTypedefPtr;

} // model
} // cc

#endif /* MODEL_CPPTYPEDEF_H_ */
