AM_CPPFLAGS+=-I$(srcdir)/service/search-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/search-api/include/search-api
lib_LTLIBRARIES+=lib/libsearch-api.la
lib_libsearch_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

lib_libsearch_api_la_SOURCES=\
	service/search-api/src/search_constants.cpp\
	service/search-api/include/search-api/search_constants.h\
	service/search-api/src/search_types.cpp\
	service/search-api/include/search-api/search_types.h\
	service/search-api/src/SearchService.cpp\
	service/search-api/include/search-api/SearchService.h

SEARCH_API_JAVA_SRC=\
	service/search-api/gen-java/cc/service/search/SearchException.java \
	service/search-api/gen-java/cc/service/search/SearchFilter.java \
	service/search-api/gen-java/cc/service/search/SearchOptions.java  \
	service/search-api/gen-java/cc/service/search/SearchParams.java \
	service/search-api/gen-java/cc/service/search/SearchQuery.java \
	service/search-api/gen-java/cc/service/search/SearchRange.java \
	service/search-api/gen-java/cc/service/search/SearchService.java  \
	service/search-api/gen-java/cc/service/search/SearchType.java \
	service/search-api/gen-java/cc/service/search/SearchSuggestionParams.java \
	service/search-api/gen-java/cc/service/search/SearchSuggestions.java

$(SEARCH_API_JAVA_SRC): search-api-rule 

$(lib_libsearch_api_la_SOURCES): search-api-rule 

$(srcdir)/search-api-rule: service/search-api/search.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/search-api/include/search-api
	mkdir --parents $(srcdir)/service/search-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/search-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/search-api $<
	thrift --gen java -I $(CCTHRIFT_INCLUDE) -o service/search-api $<
	cp -f $(srcdir)/service/search-api/gen-cpp/*.h $(srcdir)/service/search-api/include/search-api
	cp -f $(srcdir)/service/search-api/gen-cpp/*.cpp $(srcdir)/service/search-api/src
	rm  $(srcdir)/service/search-api/gen-cpp/SearchService_server.skeleton.cpp
	touch search-api-rule
	
#copying javascripts file into webroot
searchapijsdir=$(jsdir)/search-api
searchapijs_DATA=\
	service/search-api/gen-js/SearchService.js\
	service/search-api/gen-js/search_types.js

$(searchapijs_DATA) : search-api-rule

EXTRA_DIST+=service/search-api/search.thrift search-api-rule\
  $(searchapijs_DATA)

CLEANFILES+=service/search-api/gen-cpp/SearchService_server.skeleton.cpp

GENCLEANFILES+=$(lib_libsearch_api_la_SOURCES) search-api-rule \
  $(searchapijs_DATA) $(SEARCH_API_JAVA_SRC)


install-data-hook::
	mkdir -p ${thriftapidir}/search-api
	cp -R $(srcdir)/service/search-api/*.thrift ${thriftapidir}/search-api
