AM_CPPFLAGS+=-I$(srcdir)/service/version-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/version-api/include/version-api
lib_LTLIBRARIES+=lib/libversion-api.la
lib_libversion_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

lib_libversion_api_la_SOURCES=\
	service/version-api/src/version_constants.cpp\
	service/version-api/include/version-api/version_constants.h\
	service/version-api/src/version_types.cpp\
	service/version-api/include/version-api/version_types.h\
	service/version-api/src/VersionService.cpp\
	service/version-api/include/version-api/VersionService.h

$(version_API_JAVA_SRC): version-api-rule 

$(lib_libversion_api_la_SOURCES): version-api-rule 

$(srcdir)/version-api-rule: service/version-api/version.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/version-api/include/version-api
	mkdir --parents $(srcdir)/service/version-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/version-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/version-api $<
	cp -f $(srcdir)/service/version-api/gen-cpp/*.h $(srcdir)/service/version-api/include/version-api
	cp -f $(srcdir)/service/version-api/gen-cpp/*.cpp $(srcdir)/service/version-api/src
	rm  $(srcdir)/service/version-api/gen-cpp/VersionService_server.skeleton.cpp
	touch version-api-rule
	
#copying javascripts file into webroot
versionapijsdir=$(jsdir)/version-api
versionapijs_DATA=\
	service/version-api/gen-js/VersionService.js\
	service/version-api/gen-js/version_types.js

$(versionapijs_DATA) : version-api-rule

EXTRA_DIST+=service/version-api/version.thrift version-api-rule\
  $(versionapijs_DATA)

CLEANFILES+=service/version-api/gen-cpp/VersionService_server.skeleton.cpp

GENCLEANFILES+=$(lib_libversion_api_la_SOURCES) version-api-rule \
  $(versionapijs_DATA) $(version_API_JAVA_SRC)


install-data-hook::
	mkdir -p ${thriftapidir}/version-api
	cp -R $(srcdir)/service/version-api/*.thrift ${thriftapidir}/version-api

