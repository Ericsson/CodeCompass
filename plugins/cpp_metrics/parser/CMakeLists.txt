include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/model/include
  ${PROJECT_BINARY_DIR}/plugins/cpp/model/include
  ${PLUGIN_DIR}/model/include)

add_library(cppmetricsparser SHARED
  src/cppmetricsparser.cpp)

target_link_libraries(cppmetricsparser
  cppmetricsmodel
  model
  util
  ${Boost_LIBRARIES})

install(TARGETS cppmetricsparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR})
