set(ODB_SOURCES
  include/model/microservice.h
  include/model/yamlastnode.h
  include/model/yamlfile.h
  include/model/yamlcontent.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(yamlmodel ${ODB_CXX_SOURCES})
add_dependencies(yamlmodel model)

install_sql()
