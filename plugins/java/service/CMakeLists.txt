include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp
      -o ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/java.thrift
  DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/java.thrift
  COMMENT
    "Generating Thrift for java.thrift")

add_library(javathrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.cpp)

target_compile_options(javathrift PUBLIC -fPIC)

add_library(javaservice SHARED
        src/javaservice.cpp
  src/plugin.cpp)

target_compile_options(javaservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(javaservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  javathrift)

install(TARGETS javaservice DESTINATION ${INSTALL_SERVICE_DIR})
