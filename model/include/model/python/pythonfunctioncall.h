#ifndef MODEL_PTHON_PYTHONFUNCTIONCALL_H
#define MODEL_PTHON_PYTHONFUNCTIONCALL_H

#include <string>

#include <odb/core.hxx>

#include <model/python/pythonastnode.h>

namespace cc
{
namespace model
{

#pragma db object
struct PythonFunctionCall
{
  friend class odb::access;

  typedef uint64_t pktype;
  

  #pragma db id
  pktype id;

  uint32_t arg_num;

#ifndef NO_INDICES
  #pragma db index ("PythonFunctionCall_id_i") member(id)
#endif
};

#pragma db view \
  object(PythonFunctionCall) \
  object(PythonAstNode: PythonFunctionCall::id == PythonAstNode::id)
struct PythonFunctionCallWithName
{
  typedef uint64_t pktype;

  #pragma db column(PythonFunctionCall::id)
  pktype id;

  #pragma db column(PythonAstNode::name)
  std::string name;

  #pragma db column(PythonFunctionCall::arg_num)
  uint32_t arg_num;
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONFUNCTIONCALL_H
