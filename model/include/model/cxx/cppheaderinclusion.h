/*
 * headerinclusion.h
 *
 *  Created on: Jul 9, 2013
 *      Author: ezoltbo
 */

#ifndef HEADERINCLUSION_H_
#define HEADERINCLUSION_H_

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include "model/file.h"

namespace cc
{
namespace model
{


#pragma db object
struct CppHeaderInclusion
{
  #pragma db id auto
  int id;

  #pragma db not_null
  odb::lazy_shared_ptr<File> includer;

  #pragma db not_null
  odb::lazy_shared_ptr<File> included;

  #pragma db not_null
  odb::lazy_shared_ptr<File> context;

#ifndef NO_INDICES
  #pragma db index member(includer)
  #pragma db index member(included)
#endif
};

typedef odb::lazy_shared_ptr<CppHeaderInclusion> CppHeaderInclusionPtr;

#pragma db view object(CppHeaderInclusion) \
  query("GROUP BY" + CppHeaderInclusion::included)
struct IncludedHeaders
{
  #pragma db column(CppHeaderInclusion::included)
  FileId file;
};

} // model
} // cc


#endif /* HEADERINCLUSION_H_ */
