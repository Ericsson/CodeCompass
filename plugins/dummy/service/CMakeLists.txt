include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(DUMMYTHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.h)

set(DUMMYTHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift)

set(DUMMYTHRIFT_ADDITIONAL_ARGS --gen cpp)

generate_from_thrift("${DUMMYTHRIFT_OUTPUT_FILES}" "${DUMMYTHRIFT_THRIFT_FILE}" "${DUMMYTHRIFT_ADDITIONAL_ARGS}")

set(dummythrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.cpp
)

add_library(dummythrift STATIC ${dummythrift_sources})

target_compile_options(dummythrift PUBLIC -fPIC)

add_library(dummyservice SHARED
  src/dummyservice.cpp
  src/plugin.cpp)

target_compile_options(dummyservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(dummyservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  dummythrift)

install(TARGETS dummyservice DESTINATION ${INSTALL_SERVICE_DIR})
