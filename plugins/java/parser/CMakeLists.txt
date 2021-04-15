set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

add_library(javaparser SHARED
  src/javaparser.cpp)

add_jar(javaparserjava
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/AstVisitor.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/Parser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/PositionInfo.java
  ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/Main.java
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/srcjava/META-INF/MANIFEST.MF
  OUTPUT_NAME javaparser)

add_dependencies(javaparserjava javaparser)

target_compile_options(javaparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS javaparser DESTINATION ${INSTALL_PARSER_DIR})
install_jar(javaparserjava "${INSTALL_JAVA_LIB_DIR}")
