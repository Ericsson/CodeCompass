/*
 * variable.h
 *
 *  Created on: May 7, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CXX_VARIABLE_H_
#define MODEL_CXX_VARIABLE_H_

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/cxx/cppentity.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppVariable : CppTypedEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  bool isGlobal = false;
};

typedef odb::lazy_shared_ptr<CppVariable> CppVariablePtr;

} // model
} // cc


#endif /* MODEL_CXX_VARIABLE_H_ */
