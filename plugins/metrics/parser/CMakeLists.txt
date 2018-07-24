map_model_include_directories(
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(metricsparser SHARED
  src/metricsparser.cpp)

find_boost_libraries(
  filesystem
  log)

target_link_libraries(metricsparser
  metricsmodel
  util
  ${Boost_LINK_LIBRARIES})

install(TARGETS metricsparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR})
