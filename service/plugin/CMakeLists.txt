include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/util/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/plugin.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin.thrift
  COMMENT
    "Generating Thrift for plugin.thrift")

add_library(pluginthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.cpp)

target_compile_options(pluginthrift PUBLIC -fPIC -Wno-unknown-pragmas)

add_library(pluginservice SHARED
  src/pluginservice.cpp
  src/plugin.cpp)

target_link_libraries(pluginservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  pluginthrift)

install(TARGETS pluginservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
