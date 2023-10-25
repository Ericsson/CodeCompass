include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(METRICSTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/MetricsService.cpp
)

set(METRICSTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/metrics.thrift)

set(METRICSTHRIFT_ADDITIONAL_ARGS --gen cpp --gen js -I ${PROJECT_SOURCE_DIR}/service)

generate_from_thrift("${METRICSTHRIFT_OUTPUT_FILES}" "${METRICSTHRIFT_THRIFT_FILE}" "${METRICSTHRIFT_ADDITIONAL_ARGS}")

set(metricsthrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/MetricsService.cpp
)
  
add_library(metricsthrift STATIC ${metricsthrift_sources})

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
  projectservice
  commonthrift
  metricsthrift)

install(TARGETS metricsservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
