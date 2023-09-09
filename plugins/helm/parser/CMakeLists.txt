find_package(yaml-cpp REQUIRED)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${CMAKE_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include
  ${YAML_CPP_INCLUDE_DIRS})

add_library(helmparser SHARED
  src/templateanalyzer.cpp
  src/valueanalyzer.cpp
  src/helmparser.cpp)

target_link_libraries(helmparser
  helmmodel
  model
  util
  ${Boost_LIBRARIES}
  ${YAML_CPP_LIBRARIES})

install(TARGETS helmparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR})
