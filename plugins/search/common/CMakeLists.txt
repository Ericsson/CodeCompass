set(CMAKE_JAVA_INCLUDE_PATH 
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(searchcommonjava
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/Tag.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/Tags.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/LineInformations.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/Location.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/SourceTextAnalyzer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/SourceTextTokenizer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/FileLoggerInitializer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/IndexFields.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/SuggestionDatabase.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/NFSFriendlyLockFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/config/UnknownArgumentException.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/config/InvalidValueException.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/config/CommonOptions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/config/LogConfigurator.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/common/ipc/IPCProcessor.java
  OUTPUT_NAME searchcommon)

install_jar(searchcommonjava "${INSTALL_JAVA_LIB_DIR}")
