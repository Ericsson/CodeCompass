set(ODB_SOURCES
  include/model/cppclassmetrics.h
  include/model/cppdirectorymetrics.h
  include/model/cppfunctionmetrics.h
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(cppmetricsmodel ${ODB_CXX_SOURCES})
target_link_libraries(cppmetricsmodel model)

install_sql()