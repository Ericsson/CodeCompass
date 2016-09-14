AM_CPPFLAGS+=-I$(srcdir)/parser/javaparser-api/include

#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/parser/javaparser-api/include/javaparser-api

lib_LTLIBRARIES+=lib/libjavaparser-api.la
lib_libjavaparser_api_la_LDFLAGS= $(AM_LDFLAGS)
lib_libjavaparser_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_libjavaparser_api_la_LIBADD= $(AM_LIBADD) -lthrift

lib_libjavaparser_api_la_SOURCES= \
  parser/javaparser-api/include/javaparser-api/javaparserprocess.h \
  parser/javaparser-api/src/javaparserprocess.cpp \
  parser/javaparser-api/src/javaparser_constants.cpp \
  parser/javaparser-api/include/javaparser-api/javaparser_constants.h \
  parser/javaparser-api/src/javaparser_types.cpp \
  parser/javaparser-api/include/javaparser-api/javaparser_types.h \
  parser/javaparser-api/src/JavaParserService.cpp \
  parser/javaparser-api/include/javaparser-api/JavaParserService.h

JAVA_PARSER_API_JAVA_SRC=\
	parser/javaparser-api/gen-java/cc/parser/JavaParserArg.java \
	parser/javaparser-api/gen-java/cc/parser/JavaParserService.java \
	parser/javaparser-api/gen-java/cc/parser/JavaParsingResult.java

$(lib_libjavaparser_api_la_SOURCES): javaparser-api-rule
$(JAVA_PARSER_API_JAVA_SRC): javaparser-api-rule

$(srcdir)/javaparser-api-rule: parser/javaparser-api/javaparser.thrift
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o parser/javaparser-api $<
	thrift --gen java -I $(CCTHRIFT_INCLUDE) -o parser/javaparser-api $<
	rm  $(srcdir)/parser/javaparser-api/gen-cpp/JavaParserService_server.skeleton.cpp
	cp -f $(srcdir)/parser/javaparser-api/gen-cpp/*.h $(srcdir)/parser/javaparser-api/include/javaparser-api
	cp -f $(srcdir)/parser/javaparser-api/gen-cpp/*.cpp $(srcdir)/parser/javaparser-api/src
	touch javaparser-api-rule

EXTRA_DIST+=parser/javaparser-api/javaparser.thrift javaparser-api-rule
GENCLEANFILES+= javaparser-api-rule $(JAVA_PARSER_API_JAVA_SRC)
