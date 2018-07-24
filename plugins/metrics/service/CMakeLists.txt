set_rpath_with_libraries(ServicePlugins)

map_model_include_directories(${PLUGIN_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_thrift_library(metrics "cpp;js" "common;project")

add_library(metricsservice SHARED
  src/metricsservice.cpp
  src/plugin.cpp)

target_link_libraries(metricsservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  model
  metricsmodel
  projectthrift
  projectservice
  commonthrift
  metricsthrift)

install(TARGETS metricsservice DESTINATION ${INSTALL_SERVICE_DIR})
