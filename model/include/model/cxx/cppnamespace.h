/*
 * cppenum.h
 *
 *  Created on: Sep 20, 2013
 *      Author: ezoltbo
 */

#ifndef MODEL_CPPNAMESPACE_H_
#define MODEL_CPPNAMESPACE_H_

#include "cppentity.h"
#include <odb/lazy-ptr.hxx>

namespace cc
{
namespace model
{

/**
 * This class represents a namespace in c++
 */
#pragma db object
struct CppNamespace : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }
};

const std::string CppGlobalNamespace = "Global-Namespace";

typedef std::shared_ptr<CppNamespace> CppNamespacePtr;

} // model
} // cc

#endif /* MODEL_CPPNAMESPACE_H_ */
