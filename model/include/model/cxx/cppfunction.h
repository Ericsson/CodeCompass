/*
 * function.h
 *
 *  Created on: May 7, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CXX_FUNCTION_H_
#define MODEL_CXX_FUNCTION_H_

#include <vector>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/cxx/cppentity.h>
#include <model/cxx/cppvariable.h>

namespace cc
{
namespace model
{

/**
 * An instance of this class should be an AST node which
 * represents a function definition
 * there will be exactly one object per function definition. //TODO: make this sentence true
 */
#pragma db object
struct CppFunction : CppTypedEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  std::vector<CppVariablePtr> parameters;
  std::vector<CppVariablePtr> locals;

  bool isVirtual = false;
};

typedef std::shared_ptr<CppFunction> CppFunctionPtr;

} // model
} // cc


#endif /* MODEL_CXX_FUNCTION_H_ */
