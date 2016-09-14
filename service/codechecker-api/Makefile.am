AM_CPPFLAGS+=-I$(srcdir)/service/codechecker-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/codechecker-api/include/codechecker-api
lib_libcodechecker_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_LTLIBRARIES+=lib/libcodechecker-api.la

CCTHRIFT_CODECHECKERSHARED_SOURCES=\
    service/codechecker-api/src/shared_constants.cpp \
    service/codechecker-api/include/codechecker-api/shared_constants.h \
    service/codechecker-api/src/shared_types.cpp \
    service/codechecker-api/include/codechecker-api/shared_types.h

CCTHRIFT_CODECHECKERCCDBACCESS_SOURCES=\
    service/codechecker-api/src/codeCheckerDBAccess_constants.cpp\
    service/codechecker-api/include/codechecker-api/codeCheckerDBAccess_constants.h\
    service/codechecker-api/src/codeCheckerDBAccess_types.cpp\
    service/codechecker-api/include/codechecker-api/codeCheckerDBAccess_types.h\
    service/codechecker-api/src/codeCheckerDBAccess.cpp\
    service/codechecker-api/include/codechecker-api/codeCheckerDBAccess.h


lib_libcodechecker_api_la_SOURCES=$(CCTHRIFT_CODECHECKERSHARED_SOURCES)\
    $(CCTHRIFT_CODECHECKERCCDBACCESS_SOURCES)

$(CCTHRIFT_CODECHECKERSHARED_SOURCES): $(srcdir)/codechecker-api-shared-rule

$(srcdir)/codechecker-api-shared-rule: service/codechecker-api/shared.thrift
	mkdir --parents $(srcdir)/service/codechecker-api/include/codechecker-api
	mkdir --parents $(srcdir)/service/codechecker-api/src
	thrift --gen cpp -o service/codechecker-api $<
	thrift --gen js -o service/codechecker-api $<
	cp -f $(srcdir)/service/codechecker-api/gen-cpp/shared_*.h $(srcdir)/service/codechecker-api/include/codechecker-api
	cp -f $(srcdir)/service/codechecker-api/gen-cpp/shared_*.cpp $(srcdir)/service/codechecker-api/src
	touch $@

EXTRA_DIST+=service/codechecker-api/shared.thrift codechecker-api-shared-rule

$(CCTHRIFT_CODECHECKERCCDBACCESS_SOURCES) :  $(srcdir)/codechecker-api-codeCheckerDBAccess-rule

$(srcdir)/codechecker-api-codeCheckerDBAccess-rule: service/codechecker-api/codeCheckerDBAccess.thrift service/codechecker-api/shared.thrift
	mkdir --parents $(srcdir)/service/codechecker-api/include/codechecker-api
	mkdir --parents $(srcdir)/service/codechecker-api/src
	thrift --gen cpp -o service/codechecker-api $<
	thrift --gen js -o service/codechecker-api $<
	cp -f $(srcdir)/service/codechecker-api/gen-cpp/[Cc]ode[Cc]hecker[Dd][Bb][Aa]ccess*.h $(srcdir)/service/codechecker-api/include/codechecker-api
	cp -f $(srcdir)/service/codechecker-api/gen-cpp/[Cc]ode[Cc]hecker[Dd][Bb][Aa]ccess*.cpp $(srcdir)/service/codechecker-api/src
	touch $@

EXTRA_DIST+=service/codechecker-api/codeCheckerDBAccess.thrift codechecker-api-codeCheckerDBAccess-rule
CLEANFILES+=service/codechecker-api/src/codeCheckerDBAccess_server.skeleton.cpp


#copying javascripts file into webroot
codecheckerjsdir=$(jsdir)/codechecker-api
codecheckerjs_DATA=\
	service/codechecker-api/gen-js/codeCheckerDBAccess.js\
	service/codechecker-api/gen-js/codeCheckerDBAccess_types.js\
	service/codechecker-api/gen-js/shared_types.js

$(codecheckerjs_DATA): $(srcdir)/codechecker-api-codeCheckerDBAccess-rule $(srcdir)/codechecker-api-shared-rule

EXTRA_DIST+=$(codecheckerjs_DATA)

GENCLEANFILES+= $(lib_libcodechecker_api_la_SOURCES)\
	$(srcdir)/codechecker-api-shared-rule \
	$(srcdir)/codechecker-api-codeCheckerDBAccess-rule \
	$(codecheckerjs_DATA)


install-data-hook::
	mkdir -p ${thriftapidir}/codechecker-api
	cp -R $(srcdir)/service/codechecker-api/*.thrift ${thriftapidir}/codechecker-api
