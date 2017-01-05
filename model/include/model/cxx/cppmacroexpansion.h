/*
 * macroexpansion.h
 *
 *  Created on: Jul 9, 2013
 *      Author: ezoltbo
 */

#ifndef MACROEXPANSION_H_
#define MACROEXPANSION_H_

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include "cppastnode.h"

namespace cc
{
namespace model
{


#pragma db object
struct CppMacroExpansion
{
  #pragma db id auto
  int id;

  #pragma db not_null
  CppAstNodePtr astNodePtr;

  std::string expansion;

#ifndef NO_INDICES
  #pragma db index member(astNodePtr)
#endif
};

typedef odb::lazy_shared_ptr<CppMacroExpansion> CppMacroExpansionPtr;

} // model
} // cc

#endif /* MACROEXPANSION_H_ */
