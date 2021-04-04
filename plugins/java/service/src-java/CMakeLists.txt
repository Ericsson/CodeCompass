set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javaservicejava
  JavaHandler.java
  OUTPUT_NAME javaservice
  INCLUDE_JARS corethriftjava javathriftjava)

install_jar(javaservicejava "${INSTALL_JAVA_LIB_DIR}")
