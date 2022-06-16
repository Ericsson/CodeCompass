include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include
  ${YAML_CPP_INCLUDE_DIRS})

add_library(yamlparser SHARED
        src/yamlparser.cpp)

target_link_libraries(yamlparser
  yamlmodel
  model
  util
  ${Boost_LIBRARIES}
  ${YAML_CPP_LIBRARIES})

install(TARGETS yamlparser
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  DESTINATION ${INSTALL_PARSER_DIR})
