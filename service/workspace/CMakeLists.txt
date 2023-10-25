include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(WORKSPACETHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.h)

set(WORKSPACETHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/workspace.thrift)

set(WORKSPACETHRIFT_ADDITIONAL_ARGS --gen cpp --gen js)

generate_from_thrift("${WORKSPACETHRIFT_OUTPUT_FILES}" "${WORKSPACETHRIFT_THRIFT_FILE}" "${WORKSPACETHRIFT_ADDITIONAL_ARGS}")

set(workspacethrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/workspace_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/WorkspaceService.cpp)

add_library(workspacethrift STATIC ${workspacethrift_sources})

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
