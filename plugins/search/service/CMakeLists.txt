map_model_include_directories(${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

add_thrift_library(search "cpp;java;js" "common;project")

set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java
  ${PROJECT_BINARY_DIR}/service/project/gen-java)

# Create java library from thrift files
add_jar(searchthriftjava
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/DatasourceError.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/FileSearchResult.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/HitCountResult.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/LineMatch.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/RangedHitCountResult.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchException.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchFilter.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchOptions.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchParams.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchRange.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchResult.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchResultEntry.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchService.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchSuggestionParams.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchSuggestions.java
  ${CMAKE_CURRENT_BINARY_DIR}/gen-java/cc/service/search/SearchType.java
  OUTPUT_NAME searchthrift
  INCLUDE_JARS searchcommonjava)

add_dependencies(searchthriftjava searchthrift)

# Search java
add_subdirectory(search-java)

# Create services
add_library(searchservice SHARED
  src/searchservice.cpp
  src/plugin.cpp)

target_compile_options(searchservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(searchservice
  util
  model
  mongoose
  searchthrift
  projectservice
  projectthrift
  languagethrift
  ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS searchservice DESTINATION ${INSTALL_SERVICE_DIR})
install_jar(searchthriftjava ${INSTALL_JAVA_LIB_DIR})
