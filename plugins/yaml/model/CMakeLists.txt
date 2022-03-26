set(ODB_SOURCES
  include/model/yaml.h
  include/model/yamlcontent.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(yamlmodel ${ODB_CXX_SOURCES})
add_dependencies(yamlmodel model)

install_sql()
