#ifndef CC_MODEL_PYTHON_PYTHON_VARIABLE_REF_H
#define CC_MODEL_PYTHON_PYTHON_VARIABLE_REF_H

#include <string>

#include <odb/core.hxx>

#include "pythonastnode.h"

namespace cc
{
namespace model
{

/**
 * Database table for class data members, global and local variables.
 */
#pragma db object
struct PythonVariableRef
{
  /**
   * Reference type.
   */
  enum class RefType
  {
    Unset,        ///< Reference type is not set. It should be an error.
    Definition,   ///< This a definition. A definition is also a write.
    Read,         ///< Read reference.
    Write         ///< Write reference (the variable is already defined).
  };

  #pragma db id auto
  unsigned long id;

  #pragma db not_null
  std::string mangledName;

  #pragma db not_null
  odb::lazy_shared_ptr<PythonAstNode> astNode;

  #pragma db not_null
  RefType refType = RefType::Unset;

#ifndef NO_INDICES
  #pragma db index ("PythonVariableRef_astNode_i") member(astNode)
  #pragma db index ("PythonVariableRef_mangledName_i") member(mangledName)
#endif
};

#pragma db view \
  object(PythonVariableRef = Ref) \
  object(PythonAstNode = Node: Ref::astNode) \
  object(File = FileLoc: Node::location.file) \
  query((?) + \
    "GROUP BY" + FileLoc::id + "," + FileLoc::path + "," + FileLoc::filename + \
    "ORDER BY " + FileLoc::path)
struct PythonVariableRefFile
{
  #pragma db column(FileLoc::id)
  FileId file;

  #pragma db column(FileLoc::path)
  std::string path;

  #pragma db column(FileLoc::filename)
  std::string filename;
};

} // model
} // cc

#endif // CC_MODEL_PYTHON_PYTHON_VARIABLE_REF_H
