#ifndef MODEL_PTHON_PYTHONUNKNOWN_H
#define MODEL_PTHON_PYTHONUNKNOWN_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>


namespace cc
{
namespace model
{

#pragma db object
struct PythonUnknown
{
  enum Kind
  {
    Attribute = 1,
    Function = 2,
    Unknown = 3
  };

  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id;

  #pragma db null
  odb::nullable<std::string> target;

  std::string name;

  Kind kind;

#ifndef NO_INDICES
  #pragma db index ("PythonUnknown_id_i") member(id)
  #pragma db index ("PythonUnknown_target_i") member(target)
  #pragma db index ("PythonUnknown_name_i") member(name)
#endif
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONUNKNOWN_H
