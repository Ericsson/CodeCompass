#ifndef MODEL_PTHON_PYTHONDECORATOR_H
#define MODEL_PTHON_PYTHONDECORATOR_H

#include <string>

#include <odb/core.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonDecorator
{
  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id; // should refere to an AstNode

  pktype target; // should refere to a Binding

  std::string value;

#ifndef NO_INDICES
  #pragma db index ("PythonDecorator_id_i") member(id)
  #pragma db index ("PythonDecorator_target_i") member(target)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONDECORATOR_H
