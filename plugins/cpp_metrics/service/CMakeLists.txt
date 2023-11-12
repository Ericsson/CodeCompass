include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cppmetrics.thrift
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/cppmetrics.thrift
  COMMENT
    "Generating Thrift for cppmetrics.thrift")

add_library(cppmetricsthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppMetricsService.cpp)

target_compile_options(cppmetricsthrift PUBLIC -fPIC)

add_library(cppmetricsservice SHARED
  src/cppmetricsservice.cpp
  src/plugin.cpp)

target_compile_options(cppmetricsservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(cppmetricsservice
  cppmetricsmodel
  model
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  cppmetricsthrift)

install(TARGETS cppmetricsservice DESTINATION ${INSTALL_SERVICE_DIR})
