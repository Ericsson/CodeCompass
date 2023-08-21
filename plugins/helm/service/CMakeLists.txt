include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/HelmService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/HelmService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
  ${CMAKE_CURRENT_SOURCE_DIR}/helm.thrift
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/helm.thrift
  COMMENT
    "Generating Thrift for helm.thrift")

add_library(helmthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/helm_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/HelmService.cpp)

target_compile_options(helmthrift PUBLIC -fPIC)

add_dependencies(helmthrift projectthrift)
add_dependencies(helmthrift languagethrift)

add_library(helmservice SHARED
  src/helmservice.cpp
  src/helmfilediagram.cpp
  src/plugin.cpp)

target_compile_options(helmservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(helmservice
  util
  model
  helmmodel
  gvc
  projectservice
  languagethrift
  helmthrift
  ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS helmservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
