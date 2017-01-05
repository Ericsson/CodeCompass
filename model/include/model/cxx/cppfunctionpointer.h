/*
 * variable.h
 *
 *  Created on: May 7, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CXX_FUNCTIONPOINTER_H_
#define MODEL_CXX_FUNCTIONPOINTER_H_

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/cxx/cppvariable.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppFunctionPointer : CppVariable
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }
};

typedef odb::lazy_shared_ptr<CppFunctionPointer> CppFunctionPointerPtr;

} // model
} // cc


#endif /* MODEL_CXX_FUNCTIONPOINTER_H_ */
