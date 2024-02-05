include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cxxmetrics_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cxxmetrics_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
      ${CMAKE_CURRENT_SOURCE_DIR}/cxxmetrics.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/cxxmetrics.thrift
  COMMENT
    "Generating Thrift for cxxmetrics.thrift")

add_library(cxxmetricsthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cxxmetrics_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.cpp)

target_compile_options(cxxmetricsthrift PUBLIC -fPIC)

add_dependencies(cxxmetricsthrift projectthrift)

add_library(cxxmetricsservice SHARED
  src/cppmetricsservice.cpp
  src/plugin.cpp)

#target_compile_options(cxxmetricsservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(cxxmetricsservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  model
  cppmetricsmodel
  projectthrift
  projectservice
  commonthrift
  ${ODB_LIBRARIES}
  cxxmetricsthrift)

install(TARGETS cxxmetricsservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
