include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/dummy.thrift
  COMMENT
    "Generating Thrift for dummy.thrift")

add_library(csharpthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/dummy_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/DummyService.cpp)

target_compile_options(csharpthrift PUBLIC -fPIC)

add_library(csharpservice SHARED
  src/dummyservice.cpp
  src/plugin.cpp)

target_compile_options(csharpservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(csharpservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  csharpthrift)

install(TARGETS csharpservice DESTINATION ${INSTALL_SERVICE_DIR})
