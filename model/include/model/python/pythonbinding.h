#ifndef MODEl_PYTHON_PYTHONBINDING_H
#define MODEl_PYTHON_PYTHONBINDING_H

#include <string>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include <model/fileloc.h>


namespace cc
{
namespace model
{

#pragma db object
struct PythonBinding
{
  enum Kind
  {
    Attribute    = 1,      // attr accessed with "." on some other object
    Class        = 2,      // class definition
    Constructor  = 3,      // __init__ functions in classes
    Function     = 4,      // plain function
    Method       = 5,      // static or instance method
    Module       = 6,      // file
    Parameter    = 7,      // function param
    Scope        = 8,      // top-level variable ("scope" means we assume it can have attrs)
    Variable     = 9,      // local variable
    ModuleRef    = 10      // imported module reference
  };

  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id
  pktype id;

  std::string name;

  #pragma db null
  odb::nullable<std::string> formatted_qname;

  #pragma db null
  odb::nullable<std::string> mangled_name;

  Kind kind;

  std::string type;

  FileLoc location;

  std::string documentation;


#ifndef NO_INDICES
  #pragma db index ("PythonBinding_id_i") member(id)
  #pragma db index ("PythonBinding_name_i") member(name)
  #pragma db index ("PythonBinding_formatted_qname_i") member(formatted_qname)
  #pragma db index ("PythonBinding_mangled_name_i") member(mangled_name)

  #pragma db index ("PythonBinding_file_i") member(location.file)
  #pragma db index ("PythonBinding_location_i") \
    members(                                    \
      location.file,                            \
      location.range.start.line,                \
      location.range.start.column,              \
      location.range.end.line,                  \
      location.range.end.column)
#endif


  bool operator<(const PythonBinding& rhs_) const
  {
    return this->id < rhs_.id;
  }


  /**
   * @return kind of this binding in a readable form
   */
  const std::string getKind() const
  {
    switch(kind)
    {
      case Kind::Attribute:
        return "Attribute";
        break;
    
      case Kind::Class:
        return "Class";
        break;

      case Kind::Constructor:
        return "Constructor";
        break;

      case Kind::Function:
        return "Function";
        break;

      case Kind::Method:
        return "Method";
        break;

      case Kind::Module:
        return "Module";
        break;

      case Kind::Parameter:
        return "Parameter";
        break;

      case Kind::Scope:
        return "Global variable";
        break;

      case Kind::Variable:
        return "Variable";
        break;

      case Kind::ModuleRef:
        return "ModuleRef";
        break;
    }
    // make the compiler happy
    return "Unknown";
  }
};

} // model
} // cc

#endif // MODEl_PYTHON_PYTHONBINDING_H
