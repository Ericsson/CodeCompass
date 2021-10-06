set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/)

include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${CMAKE_BINARY_DIR}/model/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

# Generate thrift files
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/javaparser_constants.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/javaparser_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaParserService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-java
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen java
        -o ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/javaparser.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/javaparser.thrift
  COMMENT
    "Generating Thrift for javaparser.thrift")

# Create cpp static library from thrift files
add_library(javaparserthrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/javaparser_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/javaparser_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/JavaParserService.cpp)

target_compile_options(javaparserthrift PUBLIC -fPIC)

add_dependencies(javaparserthrift commonthrift projectthrift)

# Create java library from thrift files
add_jar(javaparserthriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/java/CompileCommand.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/java/JavaParserService.java
  OUTPUT_NAME javaparserthrift)

add_dependencies(javaparserthriftjava javaparserthrift)

# Java parser
add_subdirectory(srcjava)

add_library(javaparser SHARED
  src/javaparser.cpp)

target_link_libraries(javaparser
  model
  javaparserthrift
  ${THRIFT_LIBTHRIFT_LIBRARIES})

target_compile_options(javaparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS javaparser DESTINATION ${INSTALL_PARSER_DIR})
install_jar(javaparserthriftjava "${INSTALL_JAVA_LIB_DIR}")
