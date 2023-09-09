set(ODB_SOURCES
  include/model/helmtemplate.h
  include/model/microservice.h
  include/model/microserviceedge.h
  include/model/msresource.h
  include/model/yamlastnode.h
  include/model/yamlfile.h
  include/model/yamlcontent.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(helmmodel ${ODB_CXX_SOURCES})
add_dependencies(helmmodel model)

install_sql()
