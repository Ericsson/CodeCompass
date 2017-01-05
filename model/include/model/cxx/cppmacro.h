/*
 * cppenum.h
 *
 *  Created on: Sep 20, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CPPMACRO_H_
#define MODEL_CPPMACRO_H_

#include "cppentity.h"
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

/**
 * This class represents a macro in c++
 */
#pragma db object
struct CppMacro : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }
};

typedef std::shared_ptr<CppMacro> CppMacroPtr;

} // model
} // cc

#endif /* MODEL_CPPMACRO_H_ */
