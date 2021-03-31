include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/java.thrift
  DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/java.thrift
  COMMENT
    "Generating Thrift for java.thrift")

# Create cpp static library from thrift files
add_library(javathrift STATIC
  # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_constants.cpp
  # ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/java_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaService.cpp)

target_compile_options(javathrift PUBLIC -fPIC)

add_dependencies(javathrift commonthrift projectthrift)

set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/
  ${PROJECT_BINARY_DIR}/service/project/gen-java/)

# Create java library from thrift files
add_jar(javathriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/java/JavaService.java
  OUTPUT_NAME javathrift)

add_dependencies(javathriftjava javathrift)

# Src java
add_subdirectory(src-java)

add_library(javaservice SHARED
  src/javaservice.cpp
  src/plugin.cpp)

target_compile_options(javaservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(javaservice
  util
  model
  mongoose
  javathrift
  projectservice
  projectthrift
  languagethrift
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES})

install(TARGETS javaservice DESTINATION ${INSTALL_SERVICE_DIR})
install_jar(javathriftjava ${INSTALL_JAVA_LIB_DIR})
