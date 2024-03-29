include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/workspace.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/workspace.thrift
  COMMENT
    "Generating Thrift for workspace.thrift")

add_library(workspacethrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.cpp)

target_compile_options(workspacethrift PUBLIC -fPIC -Wno-unknown-pragmas)

add_library(workspaceservice SHARED
  src/workspaceservice.cpp
  src/plugin.cpp)

target_link_libraries(workspaceservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  workspacethrift)

install(TARGETS workspaceservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
