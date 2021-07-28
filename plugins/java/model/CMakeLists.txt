set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(javamodel
  SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/EMFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/SymbolType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/enums/AstType.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaAnnotation.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaAstNode.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaConstructor.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaDocComment.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEntity.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEnum.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaEnumConstant.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaMethod.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaImport.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaInheritance.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaRecord.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaTypedEntity.java
  ${CMAKE_CURRENT_SOURCE_DIR}/JavaVariable.java
  META-INF/persistence.xml
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF
  OUTPUT_NAME javamodel)

install_jar(javamodel "${INSTALL_JAVA_LIB_DIR}")
