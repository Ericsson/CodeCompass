SEARCHSERVICE_JAVA_BASEDIR=search/service
SEARCHSERVICE_JAVA_SRC=\
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/service/ServiceApp.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/service/SearchHandler.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/service/ServiceAppOptions.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/SearchAppCommon.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/query/QueryApp.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/service/app/query/QueryAppOptions.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/AdvancedTagQueryParser.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/QueryAnalyzer.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/log/LogQueryBuilder.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/query/HIPDQuery.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/query/MatchCollector.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/query/MatchingDocEnum.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/analysis/query/SimpleMatchCollector.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/QueryContext.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/Context.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/ResultMatcher.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/MasterMatcherFactory.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/ResultMatcherFactory.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/SourceLineMatcherFactory.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/TagKindMatcherFactory.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/OffsetBasedLineMatcher.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/match/matcher/LogQueryMatcherFactory.java \
	$(SEARCHSERVICE_JAVA_BASEDIR)/src/cc/search/suggestion/SuggestionHandler.java

lib/java/searchservice.jar: lib/java/searchcommon.jar $(SEARCHSERVICE_JAVA_SRC) $(CORE_API_JAVA_SRC) $(SEARCH_API_JAVA_SRC)
	mkdir -p $(SEARCHSERVICE_JAVA_BASEDIR)/bin
	$(JAVAC) $(JAVAC_FLAGS) -cp $(JAVA_BUILD_CP):lib/java/searchcommon.jar:$(SEARCHSERVICE_JAVA_BASEDIR)/bin \
	  -d $(SEARCHSERVICE_JAVA_BASEDIR)/bin $(SEARCHSERVICE_JAVA_SRC) \
	  -Xlint:deprecation \
	  $(CORE_API_JAVA_SRC) $(SEARCH_API_JAVA_SRC)
	jar cfmv lib/java/searchservice.jar \
	  $(SEARCHSERVICE_JAVA_BASEDIR)/src/META-INF/MANIFEST.MF \
	  -C $(SEARCHSERVICE_JAVA_BASEDIR)/bin cc

$(SEARCHSERVICE_JAVA_BASEDIR)/searchquery: lib/java/searchservice.jar
	cp $(SEARCHSERVICE_JAVA_BASEDIR)/searchquery.in $(SEARCHSERVICE_JAVA_BASEDIR)/searchquery

$(SEARCHSERVICE_JAVA_BASEDIR)/searchservice: lib/java/searchservice.jar
	cp $(SEARCHSERVICE_JAVA_BASEDIR)/searchservice.in $(SEARCHSERVICE_JAVA_BASEDIR)/searchservice


bin_SCRIPTS+=$(SEARCHSERVICE_JAVA_BASEDIR)/searchquery $(SEARCHSERVICE_JAVA_BASEDIR)/searchservice

javalib_DATA+=lib/java/searchservice.jar

CLEANFILES+=\
	$(SEARCHSERVICE_JAVA_BASEDIR)/searchquery \
	$(SEARCHSERVICE_JAVA_BASEDIR)/searchservice \
	lib/java/searchservice.jar

clean-local::
	-rm -rf $(SEARCHSERVICE_JAVA_BASEDIR)/bin/cc
