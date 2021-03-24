set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javamodel
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaAnnotation.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaAstNode.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaDocComment.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEntity.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEnum.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEnum_enumConstants.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEnumConstant.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaFunction.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaFunction_locals.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaFunction_parameters.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaImport.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaTypedef.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaTypedEntity.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaVariable.java
  OUTPUT_NAME javamodel)

install_jar(javamodel "${INSTALL_JAVA_LIB_DIR}")