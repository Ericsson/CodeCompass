set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*)

add_jar(searchindexerjava
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/MonoTokenizer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/SourceAnalyzer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/CTags.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/TagStream.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/TagGenerator.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/TagGeneratorManager.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/tags/SourceTagGenerator.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/AbstractIndexer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/FileIndexer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/FieldReIndexer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/Context.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/IndexerTask.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/app/Options.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/app/Indexer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/indexer/util/IOHelper.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/suggestion/DatabaseBuilder.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/suggestion/DocumentIterator.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/suggestion/TagInputIterator.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/suggestion/UniqueInputIterator.java
  OUTPUT_NAME searchindexer
  INCLUDE_JARS searchcommonjava searchindexerthriftjava)

install_jar(searchindexerjava ${INSTALL_JAVA_LIB_DIR})