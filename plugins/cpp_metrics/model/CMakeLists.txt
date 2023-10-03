set(ODB_SOURCES
  include/model/cppastnodemetrics.h
  include/model/cppdirectorymetrics.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(cppmetricsmodel ${ODB_CXX_SOURCES})
target_link_libraries(cppmetricsmodel cppmodel)

install_sql()