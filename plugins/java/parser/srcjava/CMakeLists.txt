set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javaparserjava
  SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/ValidCommands.java
  ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/AstVisitor.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaParser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaParserServer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/PositionInfo.java
  ${CMAKE_CURRENT_SOURCE_DIR}/RelationCollector.java
  ${CMAKE_CURRENT_SOURCE_DIR}/Utils.java
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF
  INCLUDE_JARS javalogger javamodel javaparserthriftjava
  OUTPUT_NAME javaparser)

install_jar(javaparserjava "${INSTALL_JAVA_LIB_DIR}")