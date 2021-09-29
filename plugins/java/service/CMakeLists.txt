include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

# Generate thrift files
add_custom_command(
  OUTPUT
    # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.cpp
    # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.h
    # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.cpp
    # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-java
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen java
      -o ${CMAKE_CURRENT_BINARY_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/javaservice.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/javaservice.thrift
  COMMENT
    "Generating Thrift for javaservice.thrift")

# Create cpp static library from thrift files
add_library(javaservicethrift STATIC
  # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.cpp
  # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.cpp)

target_compile_options(javaservicethrift PUBLIC -fPIC)

add_dependencies(javaservicethrift commonthrift projectthrift)

set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/
  ${PROJECT_BINARY_DIR}/service/project/gen-java/)

# Create java library from thrift files
add_jar(javaservicethriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/java/JavaService.java
  OUTPUT_NAME javaservicethrift)

add_dependencies(javaservicethriftjava javaservicethrift)

# Src java
add_subdirectory(srcjava)

add_library(javaservice SHARED
  src/javaservice.cpp
  src/plugin.cpp)

target_compile_options(javaservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(javaservice
  util
  model
  mongoose
  javaservicethrift
  projectservice
  projectthrift
  languagethrift
  gvc
  ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS javaservice DESTINATION ${INSTALL_SERVICE_DIR})
install_jar(javaservicethriftjava ${INSTALL_JAVA_LIB_DIR})
