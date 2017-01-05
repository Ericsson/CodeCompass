/*
 * cppenum.h
 *
 *  Created on: Sep 20, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CPPENUM_H_
#define MODEL_CPPENUM_H_

#include "cppentity.h"
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

struct CppEnum;
struct CppEnumConstant;

typedef odb::lazy_shared_ptr<CppEnum> CppEnumPtr;
typedef odb::lazy_shared_ptr<CppEnumConstant> CppEnumConstantPtr;

/**
 * In an enum definition like enum A {a, b} this
 * class represents either a or b
 */
#pragma db object
struct CppEnumConstant : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  int value;
};

/**
 * This class represents an enum definition
 */
#pragma db object
struct CppEnum : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  #pragma db null
  std::vector<CppEnumConstantPtr> enumConstants;
};

} // model
} // cc

#endif /* CPPENUM_H_ */
