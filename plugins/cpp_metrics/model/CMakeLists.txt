include_directories(
  include
  ${cpp_PLUGIN_DIR}/model/include)

set(ODB_SOURCES
  include/model/cppastnodemetrics.h
  include/model/cppcohesionmetrics.h
  include/model/cppfilemetrics.h)

generate_odb_files("${ODB_SOURCES}" "cpp")

add_odb_library(cppmetricsmodel ${ODB_CXX_SOURCES})
target_link_libraries(cppmetricsmodel cppmodel)

install_sql()
