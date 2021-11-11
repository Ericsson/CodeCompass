set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javaservicejava
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/DiagramType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/FileReferenceType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/ReferenceType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaQueryFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaQueryHandler.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaQueryServer.java
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF
  OUTPUT_NAME javaservice
  INCLUDE_JARS javalogger javamodel corethriftjava javaservicethriftjava)

install_jar(javaservicejava "${INSTALL_JAVA_LIB_DIR}")
