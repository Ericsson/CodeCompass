include_directories(
  include
  plugin-api
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

list(APPEND SOURCES
  src/pluginservice.cpp
  src/plugin.cpp
)

set(GEN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin-api/gen-cpp/plugin_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin-api/gen-cpp/plugin_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin-api/gen-cpp/PluginService.cpp)
  
set_source_files_properties(${GEN_SRC} PROPERTIES GENERATED TRUE)

add_library(pluginservice SHARED ${SOURCES} ${GEN_SRC})
target_compile_options(pluginservice PUBLIC -Wno-unknown-pragmas)

set(_langs cpp js)
set(_thriftFiles ${CMAKE_CURRENT_SOURCE_DIR}/plugin.thrift)
generate_thrift_files("${_thriftFiles}" "${_langs}" pluginservice)

target_link_libraries(pluginservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}
)

install(TARGETS pluginservice DESTINATION ${INSTALL_SERVICE_DIR})