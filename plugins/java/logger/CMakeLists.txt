set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javalogger
  SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logger.java
  META-INF/logging.properties
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF
  OUTPUT_NAME javalogger)

install_jar(javalogger "${INSTALL_JAVA_LIB_DIR}")