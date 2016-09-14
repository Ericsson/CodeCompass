#ifndef MODEL_PTHON_PYTHONFUNCTIONPARAM_H
#define MODEL_PTHON_PYTHONFUNCTIONPARAM_H

#include <string>

#include <odb/core.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonFunctionParam
{
  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id; // should refere to an AstNode

  pktype target; // should refere to a FunctionDef


#ifndef NO_INDICES
  #pragma db index ("PythonFunctionParam_id_i") member(id)
  #pragma db index ("PythonFunctionParam_target_i") member(target)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONFUNCTIONPARAM_H
