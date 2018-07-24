set_rpath_with_libraries(ODB ServicePlugins)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp)

add_thrift_library(git "cpp;js" "")

add_library(gitservice SHARED
  src/plugin.cpp
  src/gitservice.cpp)

target_compile_options(gitservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(gitservice
  util
  projectservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  gitthrift
  git2)

install(TARGETS gitservice DESTINATION ${INSTALL_SERVICE_DIR})
