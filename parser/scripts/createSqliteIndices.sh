#!/bin/bash

if [ $# -eq 0 ] || [ $1 = "--help" ] || [ $1 = "-h" ];
  then
    echo "Usage: $0 /path/to/database.sqlite"
    exit 1
fi

echo "
CREATE INDEX \"treeOid_treeOrdinal_i\"
  ON \"version_treeentry\" (
    \"treeOid\",
    \"treeOrdinal\");
CREATE INDEX \"version_commit_oid_i\"
  ON \"version_commit\" (\"oid\");
CREATE INDEX \"version_commit_commitParentOids_object_id_i\"
  ON \"version_commit_commitParentOids\" (\"object_id\");
CREATE INDEX \"version_commit_commitParentOids_index_i\"
  ON \"version_commit_commitParentOids\" (\"index\");
CREATE INDEX \"version_branch_ref_i\"
  ON \"version_branch\" (\"ref\");
CREATE INDEX \"version_blob_oid_i\"
  ON \"version_blob\" (\"oid\");
CREATE INDEX \"DocComment_contentHash_i\"
  ON \"DocComment\" (\"contentHash\");
CREATE INDEX \"DocComment_mangledNameHash_i\"
  ON \"DocComment\" (\"mangledNameHash\");
CREATE INDEX \"MiAttribute_fromMi_i\"
  ON \"MiAttribute\" (\"fromMi\");
CREATE INDEX \"MiRelation_fromMi_i\"
  ON \"MiRelation\" (\"fromMi\");
CREATE INDEX \"MiRelation_toMi_i\"
  ON \"MiRelation\" (\"toMi\");
CREATE INDEX \"CppRelation_lhs_i\"
  ON \"CppRelation\" (\"lhs\");
CREATE INDEX \"CppRelation_rhs_i\"
  ON \"CppRelation\" (\"rhs\");
CREATE INDEX \"CppEnum_enumConstants_object_id_i\"
  ON \"CppEnum_enumConstants\" (\"object_id\");
CREATE INDEX \"CppEnum_enumConstants_index_i\"
  ON \"CppEnum_enumConstants\" (\"index\");
CREATE INDEX \"CppHeaderInclusion_includer_i\"
  ON \"CppHeaderInclusion\" (\"includer\");
CREATE INDEX \"CppHeaderInclusion_included_i\"
  ON \"CppHeaderInclusion\" (\"included\");
CREATE INDEX \"CppMacroExpansion_astNodePtr_i\"
  ON \"CppMacroExpansion\" (\"astNodePtr\");
CREATE INDEX \"CppType_fields_object_id_i\"
  ON \"CppType_fields\" (\"object_id\");
CREATE INDEX \"CppType_fields_index_i\"
  ON \"CppType_fields\" (\"index\");
CREATE INDEX \"CppType_functions_object_id_i\"
  ON \"CppType_functions\" (\"object_id\");
CREATE INDEX \"CppType_functions_index_i\"
  ON \"CppType_functions\" (\"index\");
CREATE INDEX \"CppFriendship_target_i\"
  ON \"CppFriendship\" (\"target\");
CREATE INDEX \"CppFriendship_theFriend_i\"
  ON \"CppFriendship\" (\"theFriend\");
CREATE INDEX \"CppInheritance_derived_i\"
  ON \"CppInheritance\" (\"derived\");
CREATE INDEX \"CppInheritance_base_i\"
  ON \"CppInheritance\" (\"base\");
CREATE INDEX \"CppFunction_parameters_object_id_i\"
  ON \"CppFunction_parameters\" (\"object_id\");
CREATE INDEX \"CppFunction_parameters_index_i\"
  ON \"CppFunction_parameters\" (\"index\");
CREATE INDEX \"CppFunction_locals_object_id_i\"
  ON \"CppFunction_locals\" (\"object_id\");
CREATE INDEX \"CppFunction_locals_index_i\"
  ON \"CppFunction_locals\" (\"index\");
CREATE INDEX \"CppEntity_astNodePtr_i\"
  ON \"CppEntity\" (\"astNodePtr\");
CREATE INDEX \"CppEntity_mangledNameHash_i\"
  ON \"CppEntity\" (\"mangledNameHash\");
CREATE INDEX \"location_file_idx\"
  ON \"CppAstNode\" (\"location_file\");
CREATE INDEX \"mangledNameHash_astType_idx\"
  ON \"CppAstNode\" (
    \"mangledNameHash\",
    \"astType\");
CREATE INDEX \"astType_symbolType_idx\"
  ON \"CppAstNode\" (
    \"astType\",
    \"symbolType\");
CREATE INDEX \"File_path_i\"
  ON \"File\" (\"path\");
CREATE INDEX \"File_content_i\"
  ON \"File\" (\"content\");
CREATE INDEX \"File_parent_i\"
  ON \"File\" (\"parent\");
CREATE INDEX \"PythonAstNode_id_i\"                 
  ON \"PythonAstNode\" (\"id\");
CREATE INDEX \"PythonAstNode_name_i\"               
  ON \"PythonAstNode\" (\"name\");
CREATE INDEX \"PythonAstNode_abv_qname_i\"          
  ON \"PythonAstNode\" (\"abv_qname\");
CREATE INDEX \"PythonAstNode_base_binding_i\"       
  ON \"PythonAstNode\" (\"base_binding\");
CREATE INDEX \"PythonAstNode_container_binding_i\"  
  ON \"PythonAstNode\" (\"container_binding\");
CREATE INDEX \"PythonAstNode_global_write_i\"       
  ON \"PythonAstNode\" (\"global_write\");
CREATE INDEX \"PythonAstNode_file_i\"               
  ON \"PythonAstNode\" (\"location_file\");
CREATE INDEX \"PythonAstNode_location_i\"           
  ON \"PythonAstNode\" (                                   
    \"location_file\",                            
    \"location_range_start_line\",                
    \"location_range_start_column\",              
    \"location_range_end_line\",                  
    \"location_range_end_column\");
CREATE INDEX \"PythonAttribute_target_i\"           
  ON \"PythonAttribute\" (\"target\");
CREATE INDEX \"PythonBinding_id_i\"                 
  ON \"PythonBinding\" (\"id\");
CREATE INDEX \"PythonBinding_name_i\"               
  ON \"PythonBinding\" (\"name\");
CREATE INDEX \"PythonBinding_formatted_qname_i\"    
  ON \"PythonBinding\" (\"formatted_qname\");
CREATE INDEX \"PythonBinding_mangled_name_i\"       
  ON \"PythonBinding\" (\"mangled_name\");
CREATE INDEX \"PythonBinding_file_i\"               
  ON \"PythonBinding\" (\"location_file\");
CREATE INDEX \"PythonBinding_location_i\"           
  ON \"PythonBinding\" (                             
    \"location_file\",                            
    \"location_range_start_line\",                
    \"location_range_start_column\",              
    \"location_range_end_line\",                  
    \"location_range_end_column\");
CREATE INDEX \"PythonClassDef_target_i\"       
  ON \"PythonClassDef\" (\"target\");
CREATE INDEX \"PythonClassDef_constructor_i\"       
  ON \"PythonClassDef\" (\"constructor\");
CREATE INDEX \"PythonDecorator_id_i\"               
  ON \"PythonDecorator\" (\"id\");
CREATE INDEX \"PythonDecorator_target_i\"           
  ON \"PythonDecorator\" (\"target\");
CREATE INDEX \"PythonFunctionCall_id_i\"            
  ON \"PythonFunctionCall\" (\"id\");
CREATE INDEX \"PythonFunctionDef_id_i\"             
  ON \"PythonFunctionDef\" (\"id\");
CREATE INDEX \"PythonFunctionParam_id_i\"           
  ON \"PythonFunctionParam\" (\"id\");
CREATE INDEX \"PythonFunctionParam_target_i\"       
  ON \"PythonFunctionParam\" (\"target\");
CREATE INDEX \"PythonInheritance_target_i\"         
  ON \"PythonInheritance\" (\"target\");
CREATE INDEX \"PythonInheritance_base_i\"           
  ON \"PythonInheritance\" (\"base\");
CREATE INDEX \"PythonReference_node_i\"             
  ON \"PythonReference\" (\"node\");
CREATE INDEX \"PythonReference_binding_i\"          
  ON \"PythonReference\" (\"binding\");
CREATE INDEX \"PythonUnknown_id_i\"                 
  ON \"PythonUnknown\" (\"id\");
CREATE INDEX \"PythonUnknown_target_i\"             
  ON \"PythonUnknown\" (\"target\");
CREATE INDEX \"PythonUnknown_name_i\"               
  ON \"PythonUnknown\" (\"name\");
CREATE INDEX \"PythonVariable_id_i\"                
  ON \"PythonVariable\" (\"id\");
CREATE INDEX \"PythonVariable_target_i\"            
  ON \"PythonVariable\" (\"target\");
CREATE INDEX \"PythonVariableRef_astNode_i\"            
  ON \"PythonVariableRef\" (\"astNode\");
CREATE INDEX \"PythonVariableRef_mangledName_i\"            
  ON \"PythonVariableRef\" (\"mangledName\");
" | sqlite3 "$1" 2>&1

