set(CMAKE_JAVA_INCLUDE_PATH
  ${PROJECT_SOURCE_DIR}/lib/java/*
  ${PLUGIN_DIR}/lib/java/*)

add_jar(searchservicejava
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/service/ServiceApp.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/service/SearchHandler.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/service/ServiceAppOptions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/SearchAppCommon.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/query/QueryApp.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/service/app/query/QueryAppOptions.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/AdvancedTagQueryParser.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/QueryAnalyzer.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/log/LogQueryBuilder.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/query/HIPDQuery.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/query/MatchCollector.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/query/MatchingDocEnum.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/analysis/query/SimpleMatchCollector.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/QueryContext.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/Context.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/ResultMatcher.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/MasterMatcherFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/ResultMatcherFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/SourceLineMatcherFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/TagKindMatcherFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/OffsetBasedLineMatcher.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/match/matcher/LogQueryMatcherFactory.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/search/suggestion/SuggestionHandler.java
  OUTPUT_NAME searchservice
  INCLUDE_JARS corethriftjava searchcommonjava searchthriftjava)

install_jar(searchservicejava "${INSTALL_JAVA_LIB_DIR}")
