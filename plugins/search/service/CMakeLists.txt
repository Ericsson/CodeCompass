include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/service/language/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/gen-cpp
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

# Generate thrift files
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/search_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/search_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/SearchService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-java
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js --gen java
      -o ${CMAKE_CURRENT_SOURCE_DIR}
      -I ${PROJECT_SOURCE_DIR}/service
      ${CMAKE_CURRENT_SOURCE_DIR}/search.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/search.thrift
  COMMENT
    "Generating Thrift for search.thrift")

# Create cpp static library from thrift files
add_library(searchthrift STATIC
  gen-cpp/search_constants.cpp
  gen-cpp/search_types.cpp
  gen-cpp/SearchService.cpp)

target_compile_options(searchthrift PUBLIC -fPIC)

add_dependencies(searchthrift commonthrift projectthrift)

set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/service/gen-java/
  ${PROJECT_SOURCE_DIR}/service/project/gen-java/)

# Create java library from thrift files
add_jar(searchthriftjava
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/DatasourceError.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/FileSearchResult.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/HitCountResult.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/LineMatch.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/RangedHitCountResult.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchException.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchFilter.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchOptions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchParams.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchRange.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchResult.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchResultEntry.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchService.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchSuggestionParams.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchSuggestions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/gen-java/cc/service/search/SearchType.java
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
install_js_thrift(gen-js/)
