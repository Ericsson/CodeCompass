set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${CMAKE_BINARY_DIR}/model/include)

add_library(javaparser SHARED
  src/javaparser.cpp)

add_jar(javaparserjava
  SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/enums/ValidCommands.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/ArgParser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/AstVisitor.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/JavaParser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/PositionInfo.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/Main.java
  META-INF/logging.properties
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF
  INCLUDE_JARS javamodel
  OUTPUT_NAME javaparser)

add_dependencies(javaparserjava javaparser)

target_link_libraries(javaparser
  model)

target_compile_options(javaparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS javaparser DESTINATION ${INSTALL_PARSER_DIR})
install_jar(javaparserjava "${INSTALL_JAVA_LIB_DIR}")
