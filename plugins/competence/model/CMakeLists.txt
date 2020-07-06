set(ODB_SOURCES
  include/model/filecomprehension.h
  include/model/commitdata.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(competencemodel ${ODB_CXX_SOURCES})
add_dependencies(competencemodel model)

install_sql()
