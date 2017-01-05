#ifndef MODEL_PTHON_PYTHONCLASSDEF_H
#define MODEL_PTHON_PYTHONCLASSDEF_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonClassDef
{
  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id auto
  pktype id;

  pktype target;

  #pragma db null
  odb::nullable<pktype> constructor;

#ifndef NO_INDICES
  #pragma db index ("PythonClassDef_target_i") member(target)
  #pragma db index ("PythonClassDef_constructor_i") member(constructor)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONCLASSDEF_H
