include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include)

set(SOURCES
  src/metricsparser.cpp )

add_library(metricsparser SHARED ${SOURCES})
target_link_libraries(metricsparser
  metricsmodel
  util
  boost_filesystem
  boost_log )

install(TARGETS metricsparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR} )
