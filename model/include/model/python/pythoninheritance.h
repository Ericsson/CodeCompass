#ifndef MODEL_PTHON_PYTHONINHERITENCE_H
#define MODEL_PTHON_PYTHONINHERITENCE_H

#include <string>

#include <odb/core.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonInheritance
{
  friend class odb::access;

  typedef uint64_t pktype;

  enum Kind
  {
    Simple = 1,
    Multiple,
    Missing
  };

  #pragma db id auto
  pktype id;

  pktype target;

  pktype base;

  Kind kind;


#ifndef NO_INDICES
  #pragma db index ("PythonInheritance_target_i") member(target)
  #pragma db index ("PythonInheritance_base_i") member(base)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONINHERITENCE_H
