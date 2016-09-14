AM_CPPFLAGS+=-I$(srcdir)/search/indexer-api/include

#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/search/indexer-api/include/indexer-api

lib_LTLIBRARIES+=lib/libsearch_indexer-api.la
lib_libsearch_indexer_api_la_LDFLAGS= $(AM_LDFLAGS)
lib_libsearch_indexer_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_libsearch_indexer_api_la_LIBADD= $(AM_LIBADD) -lthrift

lib_libsearch_indexer_api_la_SOURCES= \
	search/indexer-api/src/search_indexer_constants.cpp \
	search/indexer-api/include/indexer-api/search_indexer_constants.h \
	search/indexer-api/src/search_indexer_types.cpp \
	search/indexer-api/include/indexer-api/search_indexer_types.h \
	search/indexer-api/src/IndexerService.cpp \
	search/indexer-api/include/indexer-api/IndexerService.h \
	search/indexer-api/include/indexer-api/indexerprocess.h \
	search/indexer-api/src/indexerprocess.cpp
	
SEARCH_INDEXER_API_JAVA_SRC=\
	search/indexer-api/gen-java/cc/parser/search/FieldValue.java \
	search/indexer-api/gen-java/cc/parser/search/IndexerService.java \
	search/indexer-api/gen-java/cc/parser/search/Location.java \
	search/indexer-api/gen-java/cc/parser/search/search_indexerConstants.java

$(lib_libsearch_indexer_api_la_SOURCES): search_indexer-api-rule 

$(SEARCH_INDEXER_API_JAVA_SRC): search_indexer-api-rule 

$(srcdir)/search_indexer-api-rule: search/indexer-api/search_indexer.thrift
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o search/indexer-api $<
	thrift --gen java -I $(CCTHRIFT_INCLUDE) -o search/indexer-api $<
	rm  $(srcdir)/search/indexer-api/gen-cpp/IndexerService_server.skeleton.cpp
	cp -f $(srcdir)/search/indexer-api/gen-cpp/*.h $(srcdir)/search/indexer-api/include/indexer-api
	cp -f $(srcdir)/search/indexer-api/gen-cpp/*.cpp $(srcdir)/search/indexer-api/src
	touch search_indexer-api-rule

EXTRA_DIST+=search/indexer-api/search_indexer.thrift search_indexer-api-rule
CLEANFILES+=search/indexer-api/gen-cpp/SearchService_server.skeleton.cpp
GENCLEANFILES+= \
	search_indexer-api-rule \
	$(SEARCH_INDEXER_API_JAVA_SRC)
