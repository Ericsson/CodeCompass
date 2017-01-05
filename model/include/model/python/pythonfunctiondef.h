#ifndef MODEL_PTHON_PYTHONFUNCTIONDEF_H
#define MODEL_PTHON_PYTHONFUNCTIONDEF_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include <model/python/pythonbinding.h>

namespace cc
{
namespace model
{

#pragma db object
struct PythonFunctionDef
{
  friend class odb::access;

  typedef uint64_t pktype;

  #pragma db id
  pktype id; // should refere to a Binding

  #pragma db null
  odb::nullable<pktype> member_of; // should refer to a ClassDef
  
  uint32_t min_param_num;
  
  uint32_t max_param_num;

#ifndef NO_INDICES
  #pragma db index ("PythonFunctionDef_id_i") member(id)
#endif
};

#pragma db view \
  object(PythonFunctionDef) \
  object(PythonBinding: PythonFunctionDef::id == PythonBinding::id)
struct PythonFunctionDefWithName
{
  typedef uint64_t pktype;

  #pragma db column(PythonFunctionDef::id)
  pktype id;

  #pragma db column(PythonBinding::name)
  std::string name;

  #pragma db column(PythonFunctionDef::min_param_num)
  uint32_t min_param_num;
  
  #pragma db column(PythonFunctionDef::max_param_num)
  uint32_t max_param_num;
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONFUNCTIONDEF_H
