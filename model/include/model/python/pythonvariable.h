#ifndef MODEL_PTHON_PYTHONVARIABLE_H
#define MODEL_PTHON_PYTHONVARIABLE_H

#include <string>

#include <odb/core.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonVariable
{
  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id; // should refere to an AstNode

  pktype target; // should refere to a Binding

  bool is_global;

#ifndef NO_INDICES
  #pragma db index ("PythonVariable_id_i") member(id)
  #pragma db index ("PythonVariable_target_i") member(target)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONVARIABLE_H
