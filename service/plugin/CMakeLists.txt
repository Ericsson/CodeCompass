include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/util/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(PLUGINTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.h)

set(PLUGINTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/plugin.thrift)

set(PLUGINTHRIFT_ADDITIONAL_ARGS --gen cpp --gen js)

generate_from_thrift("${PLUGINTHRIFT_OUTPUT_FILES}" "${PLUGINTHRIFT_THRIFT_FILE}" "${PLUGINTHRIFT_ADDITIONAL_ARGS}")

set(pluginthrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/plugin_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/PluginService.cpp)

add_library(pluginthrift STATIC ${pluginthrift_sources})

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
