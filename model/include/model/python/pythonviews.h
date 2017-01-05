#ifndef MODEl_PYTHON_PYTHONVIEWS_H
#define MODEl_PYTHON_PYTHONVIEWS_H

#include <string>
#include <memory>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/file.h>
#include <model/position.h>

#include "pythonastnode.h"
#include "pythonbinding.h"
#include "pythonreference.h"


namespace cc
{
namespace model
{


/*
, \"binding\"," \
    "\"name\", \"mangled_name\", \"kind\"," \
    "\"location_range_start_line\", \"location_range_start_column\"," \
    "\"location_range_end_line\", \"location_range_end_column\"," \
    "\"location_file\"," \
    "\"documentation\""
*/




/*#pragma db view \
  object(PythonAstNode) \
  object(PythonBinding : PythonBinding::id == PythonAstNode::container_binding) \
  object(File : File::id == PythonAstNode::location.file)
struct PythonAstNodeWithContainerBinding
{
  typedef uint64_t pktype;

  // node
  #pragma db column(PythonAstNode::id)
  pktype node;

  #pragma db column(PythonAstNode::name)
  std::string node_name;

  #pragma db column(PythonAstNode::location.range.start.line)
  Position::postype node_location_range_start_line;

  #pragma db column(PythonAstNode::location.range.start.column)
  Position::postype node_location_range_start_column;

  #pragma db column(PythonAstNode::location.range.end.line)
  Position::postype node_location_range_end_line;

  #pragma db column(PythonAstNode::location.range.end.column)
  Position::postype node_location_range_end_column;

  #pragma db column(PythonAstNode::location.file)
  FileId node_location_file;

  #pragma db column(File::filename)
  std::string node_location_filename;

  #pragma db column(PythonAstNode::binding)
  pktype node_binding;

  // binding
  #pragma db column(PythonBinding::id)
  pktype container_binding;

  #pragma db column(PythonBinding::name)
  std::string container_binding_name;

  #pragma db column(PythonBinding::file)
  FileId binding_file;

  #pragma db column(PythonBinding::body_range.start.line)
  Position::postype binding_body_range_start_line;

  #pragma db column(PythonBinding::body_range.start.column)
  Position::postype binding_body_range_start_column;

  #pragma db column(PythonBinding::body_range.end.line)
  Position::postype binding_body_range_end_line;

  #pragma db column(PythonBinding::body_range.end.column)
  Position::postype binding_body_range_end_column;

  #pragma db column(PythonBinding::kind)
  PythonBinding::BindingKind kind;

  #pragma db column(PythonBinding::type)
  std::string type;
};


#pragma db view \
  object(PythonAstNode) \
  object(PythonBinding : PythonBinding::id == PythonAstNode::binding) \
  object(File : File::id == PythonAstNode::location.file)
struct PythonAstNodeWithBinding
{
  typedef uint64_t pktype;

  // node
  #pragma db column(PythonAstNode::id)
  pktype node;

  #pragma db column(PythonAstNode::name)
  std::string node_name;

  #pragma db column(PythonAstNode::location.range.start.line)
  Position::postype node_location_range_start_line;

  #pragma db column(PythonAstNode::location.range.start.column)
  Position::postype node_location_range_start_column;

  #pragma db column(PythonAstNode::location.range.end.line)
  Position::postype node_location_range_end_line;

  #pragma db column(PythonAstNode::location.range.end.column)
  Position::postype node_location_range_end_column;

  #pragma db column(PythonAstNode::location.file)
  FileId node_location_file;

  #pragma db column(File::filename)
  std::string node_location_filename;

  #pragma db column(PythonAstNode::binding)
  pktype node_binding;

  // binding
  #pragma db column(PythonBinding::id)
  pktype container_binding;

  #pragma db column(PythonBinding::name)
  std::string container_binding_name;

  #pragma db column(PythonBinding::file)
  FileId binding_file;

  #pragma db column(PythonBinding::body_range.start.line)
  Position::postype binding_body_range_start_line;

  #pragma db column(PythonBinding::body_range.start.column)
  Position::postype binding_body_range_start_column;

  #pragma db column(PythonBinding::body_range.end.line)
  Position::postype binding_body_range_end_line;

  #pragma db column(PythonBinding::body_range.end.column)
  Position::postype binding_body_range_end_column;

  #pragma db column(PythonBinding::kind)
  PythonBinding::BindingKind kind;

  #pragma db column(PythonBinding::type)
  std::string type;
};*/

}
}

#endif // MODEl_PYTHON_PYTHONVIEWS_H