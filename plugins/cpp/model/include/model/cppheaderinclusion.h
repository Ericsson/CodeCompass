#ifndef CC_MODEL_HEADERINCLUSION_H
#define CC_MODEL_HEADERINCLUSION_H

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

  std::string toString() const
  {
    return std::string("CppHeaderInclusion")
      .append("\nid = ").append(std::to_string(id));
  }

#ifndef NO_INDICES
  #pragma db index member(includer)
  #pragma db index member(included)
#endif
};

typedef odb::lazy_shared_ptr<CppHeaderInclusion> CppHeaderInclusionPtr;

} // model
} // cc

#endif // CC_MODEL_HEADERINCLUSION_H
