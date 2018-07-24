map_model_include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

set(ODB_SOURCES
  include/model/metrics.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(metricsmodel ${ODB_CXX_SOURCES})

add_dependencies(metricsmodel model)

install_sql(include/model/)
