include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_thrift_library(workspace "cpp;js" "")

add_library(workspaceservice SHARED
  src/workspaceservice.cpp
  src/plugin.cpp)

target_link_libraries(workspaceservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  workspacethrift)

install(TARGETS workspaceservice DESTINATION ${INSTALL_SERVICE_DIR})
