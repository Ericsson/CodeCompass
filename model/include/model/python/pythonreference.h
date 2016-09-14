#ifndef MODEL_PTHON_PYTHONREFERENCE_H
#define MODEL_PTHON_PYTHONREFERENCE_H

#include <string>

#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>

#include <model/fileloc.h>

#include "pythonastnode.h"
#include "pythonbinding.h"


namespace cc
{
namespace model
{


#pragma db object
struct PythonReference
{
  friend class odb::access;

  typedef uint64_t pktype;


  #pragma db id auto
  pktype id;

  pktype node; // should refere to an AstNode

  pktype binding; // should refere to a Binding


#ifndef NO_INDICES
  #pragma db index ("PythonReference_node_i") member(node)
  #pragma db index ("PythonReference_binding_i") member(binding)
#endif
};


#pragma db view \
  query( \
  "SELECT \
    DISTINCT node.id, \
    node.name, node.abv_qname, node.ast_type, \
    node.location_range_start_line, node.location_range_start_column, \
    node.location_range_end_line, node.location_range_end_column, \
    node.location_file, \
    node.base_binding, \
    node.container_binding, \
    node.global_write \
  FROM \
    \"PythonBinding\" b \
  INNER JOIN \"PythonReference\" \
    ON \"PythonReference\".binding = b.id AND (?) \
  INNER JOIN \"PythonBinding\" friends \
    ON friends.mangled_name = b.mangled_name \
  INNER JOIN \"PythonReference\" refs \
    ON refs.binding = friends.id \
  INNER JOIN \"PythonAstNode\" node \
    ON refs.node = node.id" \
  )
struct PythonRefNodeRefToRef
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;
  
  std::string abv_qname;

  PythonAstNode::AstType ast_type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  #pragma db null
  odb::nullable<PythonBinding::pktype> base_binding;

  #pragma db null
  odb::nullable<PythonBinding::pktype> container_binding;
  
  bool global_write;
};


#pragma db view \
  query( \
  "SELECT \
    DISTINCT node.id, \
    node.name, node.abv_qname, node.ast_type, \
    node.location_range_start_line, node.location_range_start_column, \
    node.location_range_end_line, node.location_range_end_column, \
    node.location_file, \
    node.base_binding, \
    node.container_binding, \
    node.global_write \
  FROM \
    \"PythonBinding\" b \
  INNER JOIN \"PythonReference\" \
    ON \"PythonReference\".binding = b.id AND (?) \
  INNER JOIN \"PythonBinding\" friends \
    ON friends.mangled_name = b.mangled_name \
  INNER JOIN \"PythonReference\" refs \
    ON refs.binding = friends.id \
  INNER JOIN \"PythonAstNode\" node \
    ON friends.id = node.id" \
  )
struct PythonRefNodeRefToBind
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;
  
  std::string abv_qname;

  PythonAstNode::AstType ast_type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  #pragma db null
  odb::nullable<PythonBinding::pktype> base_binding;

  #pragma db null
  odb::nullable<PythonBinding::pktype> container_binding;
  
  bool global_write;
};


#pragma db view \
  query( \
  "SELECT \
    DISTINCT node.id, \
    node.name, node.abv_qname, node.ast_type, \
    node.location_range_start_line, node.location_range_start_column, \
    node.location_range_end_line, node.location_range_end_column, \
    node.location_file, \
    node.base_binding, \
    node.container_binding, \
    node.global_write \
  FROM \
    \"PythonBinding\" \
  INNER JOIN \"PythonBinding\" friends \
    ON friends.mangled_name = \"PythonBinding\".mangled_name AND (?) \
  INNER JOIN \"PythonAstNode\" node on node.id = friends.id" \
  )
struct PythonRefNodeBindToBind
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;
  
  std::string abv_qname;

  PythonAstNode::AstType ast_type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  #pragma db null
  odb::nullable<PythonBinding::pktype> base_binding;

  #pragma db null
  odb::nullable<PythonBinding::pktype> container_binding;
  
  bool global_write;
};


#pragma db view \
  query( \
  "SELECT \
    DISTINCT n.id, \
    n.name, node.abv_qname, n.ast_type, \
    n.location_range_start_line, n.location_range_start_column, \
    n.location_range_end_line, n.location_range_end_column, \
    n.location_file, \
    n.base_binding, \
    n.container_binding, \
    n.global_write \
  FROM \
    \"PythonBinding\" \
  INNER JOIN \"PythonBinding\" friends \
    ON friends.mangled_name = \"PythonBinding\".mangled_name AND (?) \
  INNER JOIN \"PythonAstNode\" node \
    ON node.id = friends.id \
  INNER join \"PythonReference\" ref \
    ON ref.binding = friends.id \
  INNER join \"PythonAstNode\" n \
    ON n.id = ref.node" \
  )
struct PythonRefNodeBindToRef
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;
  
  std::string abv_qname;

  PythonAstNode::AstType ast_type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  #pragma db null
  odb::nullable<PythonBinding::pktype> base_binding;

  #pragma db null
  odb::nullable<PythonBinding::pktype> container_binding;
  
  bool global_write;
};

#pragma db view \
  query( \
  "SELECT DISTINCT\
    f.id, \
    f.name, f.formatted_qname, f.mangled_name, f.kind, f.type, \
    f.location_range_start_line, f.location_range_start_column, \
    f.location_range_end_line, f.location_range_end_column, \
    f.location_file, \
    f.documentation \
  FROM \
    \"PythonBinding\" b \
  INNER JOIN \"PythonReference\" \
    ON b.id = \"PythonReference\".binding AND (?) \
  INNER JOIN \"PythonBinding\" f \
    ON f.mangled_name = b.mangled_name" \
  )
struct PythonAllBindingRefToBind
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;

  #pragma db null
  odb::nullable<std::string> formatted_qname;

  #pragma db null
  odb::nullable<std::string> mangled_name;

  PythonBinding::Kind kind;

  std::string type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  std::string documentation;
};

#pragma db view \
  query( \
  "SELECT DISTINCT \
    f.id, \
    f.name, f.formatted_qname, f.mangled_name, f.kind, f.type, \
    f.location_range_start_line, f.location_range_start_column, \
    f.location_range_end_line, f.location_range_end_column, \
    f.location_file, \
    f.documentation \
  FROM \
    \"PythonBinding\" \
  INNER JOIN \"PythonBinding\" f \
    ON (?) AND f.mangled_name = \"PythonBinding\".mangled_name" \
  )
struct PythonAllBindingBindToBind
{
  typedef uint64_t pktype;

  pktype id;

  std::string name;

  #pragma db null
  odb::nullable<std::string> formatted_qname;

  #pragma db null
  odb::nullable<std::string> mangled_name;

  PythonBinding::Kind kind;

  std::string type;

  Position::postype location_range_start_line;

  Position::postype location_range_start_column;

  Position::postype location_range_end_line;

  Position::postype location_range_end_column;

  FileId location_file;

  std::string documentation;
};

} // model
} // cc

#endif // MODEL_PTHON_PYTHONREFERENCE_H
