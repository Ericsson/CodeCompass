include_directories(  
  include
  ${PROJECT_SOURCE_DIR}/model/include)

set(ODB_SOURCES
  include/model/metrics.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library_plugin(metricsmodel ${PLUGIN_NAME} ${ODB_CXX_SOURCES})
add_dependencies(metricsmodel model)

install_sql(${CMAKE_CURRENT_BINARY_DIR})
