include_directories(
  include
  workspace-api/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginable-webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

list(APPEND SOURCES
  src/workspaceservice.cpp
  src/plugin.cpp
)

set(GEN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/workspace-api/gen-cpp/workspace_constants.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/workspace-api/gen-cpp/workspace_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/workspace-api/gen-cpp/WorkspaceService.cpp)
  
set_source_files_properties(${GEN_SRC} PROPERTIES GENERATED TRUE)

add_library(workspaceservice SHARED ${SOURCES} ${GEN_SRC})
target_compile_options(workspaceservice PUBLIC -Wno-unknown-pragmas)

set(_langs cpp js)
set(_thriftFiles ${CMAKE_CURRENT_SOURCE_DIR}/workspace.thrift)
generate_thrift_files("${_thriftFiles}" "${_langs}" workspaceservice)

target_link_libraries(workspaceservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}
)

install(TARGETS workspaceservice DESTINATION ${INSTALL_SERVICE_DIR})
