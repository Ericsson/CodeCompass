include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include)

add_thrift_library(searchindexer "cpp;java" "")

# Create java library from thrift files
set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*)

add_jar(searchindexerthriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/search/FieldValue.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/search/IndexerService.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/search/Location.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/parser/search/searchindexerConstants.java
  OUTPUT_NAME searchindexerthrift)

add_dependencies(searchindexerthriftjava searchindexerthrift)

# Java indexer
add_subdirectory(indexer-java)

# Create indexer service library
add_library(indexerservice SHARED
  src/indexerprocess.cpp)

find_boost_libraries(
  filesystem
  log
  system)
target_link_libraries(indexerservice
  util
  searchindexerthrift
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${Boost_LINK_LIBRARIES})

install(TARGETS indexerservice DESTINATION ${INSTALL_LIB_DIR})
install_jar(searchindexerthriftjava ${INSTALL_JAVA_LIB_DIR})
