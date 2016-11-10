include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/gen-cpp
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/metrics_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/metrics_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/metrics_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/metrics_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/MetricsService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp --gen js 
      -o ${CMAKE_CURRENT_SOURCE_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
      ${CMAKE_CURRENT_SOURCE_DIR}/metrics.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/metrics.thrift
  COMMENT
    "Generate thrift for metrics.thrift")

add_library(metricsthrift STATIC
  gen-cpp/metrics_constants.cpp
  gen-cpp/metrics_types.cpp
  gen-cpp/MetricsService.cpp)

target_compile_options(metricsthrift PUBLIC -fPIC)

add_dependencies(metricsthrift projectthrift)

add_library(metricsservice SHARED
  src/metricsservice.cpp
  src/plugin.cpp)

target_link_libraries(metricsservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  model
  metricsmodel
  projectthrift
  commonthrift
  metricsthrift)

install(TARGETS metricsservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift(gen-js/)