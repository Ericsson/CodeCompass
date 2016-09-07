include_directories(  
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${ODB_INCLUDE_DIRS}
)

set(ODB_SOURCES
  include/model/metrics.h
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(metricsmodel ${ODB_CXX_SOURCES})