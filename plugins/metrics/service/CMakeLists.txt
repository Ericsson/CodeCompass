include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/MetricsService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
      ${CMAKE_CURRENT_SOURCE_DIR}/metrics.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/metrics.thrift
  COMMENT
    "Generating Thrift for metrics.thrift")

add_library(metricsthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/metrics_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/MetricsService.cpp)

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

if(APPLE)
  # Use Linux-like linking behaviour, dynamic libraries may contain references without implementation.
  # metricsservice references headers from the webserver without implementation
  set_target_properties(metricsservice PROPERTIES LINK_FLAGS
      "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS metricsservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
