include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include)

add_library(metricsparser SHARED
  src/metricsparser.cpp)

target_link_libraries(metricsparser
  metricsmodel
  util
  ${Boost_LIBRARIES})

if(APPLE)
  # Use Linux-like linking behaviour, dynamic libraries may contain references without implementation.
  # metricsparser references headers from the parser without implementation
  set_target_properties(metricsparser PROPERTIES LINK_FLAGS
      "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS metricsparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR})
