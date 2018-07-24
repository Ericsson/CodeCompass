include_directories(
  include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/util/include)

add_thrift_library(plugin "cpp;js" "")

add_library(pluginservice SHARED
  src/pluginservice.cpp
  src/plugin.cpp)

target_link_libraries(pluginservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  pluginthrift)

install(TARGETS pluginservice DESTINATION ${INSTALL_SERVICE_DIR})
