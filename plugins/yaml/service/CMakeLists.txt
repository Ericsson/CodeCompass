include_directories(
  include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

#[[add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/YamlService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
      ${CMAKE_CURRENT_SOURCE_DIR}/yaml.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/yaml.thrift
  COMMENT
    "Generating Thrift for yaml.thrift")

add_library(yamlthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/yaml_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/YamlService.cpp)]]

#target_compile_options(yamlthrift PUBLIC -fPIC)

#add_dependencies(yamlthrift projectthrift)

add_library(yamlservice SHARED
  src/yamlservice.cpp
  src/plugin.cpp)

target_compile_options(yamlservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(yamlservice
  util
  model
  yamlmodel
  gvc
  projectservice
  languagethrift
  #yamlthrift
  ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS yamlservice DESTINATION ${INSTALL_SERVICE_DIR})
#install_js_thrift()
