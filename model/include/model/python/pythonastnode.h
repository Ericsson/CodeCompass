#ifndef MODEL_PTHON_PYTHONASTNODE_H
#define MODEL_PTHON_PYTHONASTNODE_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include <model/fileloc.h>


namespace cc
{
namespace model
{

#pragma db object
struct PythonAstNode
{
  enum class AstType
  {
    Variable = 1,
    Function,
    Parameter,
    Class,
    Attribute,
    Decorator,
    Module,
    Bindingless,
    Unknown,
    Unresolved,
    ModuleRef,
    Call
  };

  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id;

  std::string name;
  
  std::string abv_qname;

  AstType ast_type;

  FileLoc location;

  #pragma db null
  odb::nullable<pktype> base_binding;

  #pragma db null
  odb::nullable<pktype> container_binding;
  
  bool global_write;

#ifndef NO_INDICES
  #pragma db index ("PythonAstNode_id_i") member(id)
  #pragma db index ("PythonAstNode_name_i") member(name)
  #pragma db index ("PythonAstNode_abv_qname_i") member(abv_qname)
  #pragma db index ("PythonAstNode_base_binding_i") member(base_binding)
  #pragma db index ("PythonAstNode_container_binding_i") member(container_binding)
  #pragma db index ("PythonAstNode_global_write_i") member(global_write)

  #pragma db index ("PythonAstNode_file_i") member(location.file)

  #pragma db index ("PythonAstNode_location_i") \
    members(                                    \
      location.file,                            \
      location.range.start.line,                \
      location.range.start.column,              \
      location.range.end.line,                  \
      location.range.end.column)
#endif


    bool operator<(const PythonAstNode& rhs_) const
    {
      return this->id < rhs_.id;
    }
};

#pragma db view \
  object(PythonAstNode) \
  object(File = LocFile : PythonAstNode::location.file) \
  query ((?) + "GROUP BY" + LocFile::id + "ORDER BY" + LocFile::id)
struct Python
{
  #pragma db column(LocFile::id)
  FileId file;

  #pragma db column("count(" + PythonAstNode::id + ")")
  std::size_t count;
};


} // model
} // cc

#endif // MODEL_PTHON_PYTHONASTNODE_H
