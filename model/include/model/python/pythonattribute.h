#ifndef MODEL_PTHON_PYTHONATTRIBUTE_H
#define MODEL_PTHON_PYTHONATTRIBUTE_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

namespace cc
{
namespace model
{

#pragma db object
struct PythonAttribute
{
  friend class odb::access;

  typedef uint64_t pktype;

  #pragma db id auto
  pktype id;

  pktype attribute; // should refer to an AstNode

  #pragma db null
  odb::nullable<pktype> target; // should refer to a ClassDef


#ifndef NO_INDICES
//  #pragma db index ("PythonAttribute_attribute_i") member(attribute)
  #pragma db index ("PythonAttribute_target_i") member(target)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONATTRIBUTE_H
