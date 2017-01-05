// $Id$
// Created by Aron Barath, 2015

#ifndef MODEL_CXX_IMPLICIT_H_
#define MODEL_CXX_IMPLICIT_H_

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/cxx/cppentity.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppImplicit : CppEntity
{
  void accept(CppEntityVisitor& v)
  {
    v.visit(*this);
  }

  // CppEntity::mangledNameHash; // the code segment belongs to this mangled name hash

  HashType typeHash; // the mangled name hash of the type which contains the code segment

  std::string code;
};

typedef std::shared_ptr<CppImplicit> CppImplicitPtr;

} // model
} // cc


#endif /* MODEL_CXX_IMPLICIT_H_ */
