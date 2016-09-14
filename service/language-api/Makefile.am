AM_CPPFLAGS+=-I$(srcdir)/service/language-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/language-api/include/language-api
lib_LTLIBRARIES+=lib/liblanguage-api.la

lib_liblanguage_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

lib_liblanguage_api_la_SOURCES=\
	service/language-api/src/language_constants.cpp\
	service/language-api/include/language-api/language_constants.h\
	service/language-api/src/language_types.cpp\
	service/language-api/include/language-api/language_types.h\
	service/language-api/src/LanguageService.cpp\
	service/language-api/include/language-api/LanguageService.h

$(lib_liblanguage_api_la_SOURCES): language-api-rule

$(srcdir)/language-api-rule : service/language-api/language.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/language-api/include/language-api
	mkdir --parents $(srcdir)/service/language-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/language-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/language-api $<
	cp -f $(srcdir)/service/language-api/gen-cpp/*.h $(srcdir)/service/language-api/include/language-api
	cp -f $(srcdir)/service/language-api/gen-cpp/*.cpp $(srcdir)/service/language-api/src
	rm $(srcdir)/service/language-api/src/LanguageService_server.skeleton.cpp
	touch language-api-rule

#copying javascripts file into webroot
languagejsdir=$(jsdir)/language-api
languagejs_DATA=\
	service/language-api/gen-js/LanguageService.js\
	service/language-api/gen-js/language_types.js

$(languagejs_DATA) : language-api-rule
	
EXTRA_DIST+=service/language-api/language.thrift language-api-rule \
  $(languagejs_DATA)
CLEANFILES+=service/language-api/gen-cpp/LanguageService_server.skeleton.cpp
GENCLEANFILES+=$(lib_liblanguage_api_la_SOURCES) $(srcdir)/language-api-rule \
  $(languagejs_DATA)

  
  install-data-hook::
	mkdir -p ${thriftapidir}/language-api
	cp -R $(srcdir)/service/language-api/*.thrift ${thriftapidir}/language-api
  
  
